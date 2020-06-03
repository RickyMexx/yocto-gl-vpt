//
// Implementation for Yocto/Extension.
//

//
// LICENSE:
//
// Copyright (c) 2020 -- 2020 Fabio Pellacini
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#include "yocto_extension.h"

#include <atomic>
#include <deque>
#include <future>
#include <memory>
#include <mutex>
using namespace std::string_literals;

// -----------------------------------------------------------------------------
// MATH FUNCTIONS
// -----------------------------------------------------------------------------
namespace yocto::extension {

// import math symbols for use
using math::abs;
using math::acos;
using math::atan2;
using math::clamp;
using math::cos;
using math::exp;
using math::flt_max;
using math::fmod;
using math::fresnel_conductor;
using math::fresnel_dielectric;
using math::identity3x3f;
using math::invalidb3f;
using math::log;
using math::make_rng;
using math::max;
using math::min;
using math::pif;
using math::pow;
using math::sample_discrete_cdf;
using math::sample_discrete_cdf_pdf;
using math::sample_uniform;
using math::sample_uniform_pdf;
using math::perlin_noise;
using math::perlin_ridge;
using math::perlin_turbulence;
using math::sin;
using math::sqrt;
using math::zero2f;
using math::zero2i;
using math::zero3f;
using math::zero3i;
using math::zero4f;
using math::zero4i;
using math::inverse;

using img::eval_volume;

} 

// -----------------------------------------------------------------------------
// IMPLEMENTATION FOR EXTENSION
// -----------------------------------------------------------------------------
namespace yocto::extension {

  bool has_vptvolume(const object* object) {
    return (object->density_vol != nullptr ||
	    object->emission_vol != nullptr);
  }

  bool check_bounds(vec3i vox_idx, vec3i bounds) {
    return vox_idx.x < bounds.x && vox_idx.y < bounds.y && vox_idx.z < bounds.z;
  }

  bool has_vpt_density(const object* object) {
    return object->density_vol != nullptr;
  }

  bool has_vpt_emission(const object* object) {
    return object->emission_vol != nullptr;
  }

  float eval_vpt_density(const vsdf& vsdf, const vec3f& uvw) {
    auto object = vsdf.object;
    if (!object->density_vol) return 0.0f;
    
    auto oframe   = object->frame;
    auto vol      = object->density_vol;
    auto scale    = object->scale_vol;
    auto offset   = object->offset_vol;

    auto uvl = transform_point(inverse(oframe), uvw) + offset;
    return eval_volume(*vol, uvl * scale, true, false, true) * object->density_mult;
  }

  float eval_vpt_emission(const vsdf& vsdf, const vec3f& uvw) {
    auto object = vsdf.object;
    if (!object->emission_vol) return 0.0f;

    auto oframe   = object->frame;
    auto vol      = object->emission_vol;
    auto scale    = object->scale_vol;
    auto offset   = object->offset_vol;
    auto inv_max  = 1.f / vol->max_voxel;

    auto uvl = transform_point(inverse(oframe), uvw) + offset;
    return eval_volume(*vol, uvl * scale, true, false, true) * inv_max;
  }
  // TODO(EManuele): Change rng to a pair of precomputed random numbers. (hopefully a solution)
  std::pair<float, vec3f> eval_delta_tracking(vsdf& vsdf, float max_distance, rng_state& rng,
					      const ray3f& ray) {
    // Precompute values for Monte Carlo sampling on img::volume<float>
    auto object       = vsdf.object;
    auto density_vol  = object->density_vol;
    auto emission_vol = object->emission_vol;
    auto max_density  = density_vol->max_voxel * object->density_mult;
    auto imax_density = 1.0f / max_density;
    // Majorant of sigma_t
    auto majorant_density = max_density;
    auto path_length = 0.0f;
    
    while (true) {
      path_length -= log(1 - rand1f(rng)) / majorant_density;
      if (path_length >= max_distance)
	break;
      auto current_pos = ray.o + path_length * ray.d;
      //printf("path: %f\tmax: %f\n", path_length, max_distance);
      auto density = eval_vpt_density(vsdf, current_pos) * imax_density;
      if (density > rand1f(rng)) {
	// Populate vsdf with medium interaction information and return
	vsdf.density = vec3f(density);
	return {path_length, (1.0 - vsdf.scatter)};
      } 
    }
    return {max_distance, vec3f{1}};
  }
  std::pair<float, vec3f> eval_pixar_delta(vsdf& vsdf, float max_distance, rng_state& rng,
					   const ray3f& _ray) {
    // Precompute values for Monte Carlo sampling on img::volume<float>
    auto object       = vsdf.object;
    auto density_vol  = object->density_vol;
    auto emission_vol = object->emission_vol;
    auto max_density  = density_vol->max_voxel * object->density_mult;
    auto imax_density = 1.0f / max_density;
    // Majorant of sigma_t
    auto majorant_density = max_density;
    auto path_length = 0.0f;
    auto ray = _ray;
    
    while (true) {
      auto d = ray.d;
      auto t = log(1 - rand1f(rng)) / majorant_density;;
      path_length -= t;
      if (path_length >= max_distance)
	break;
      auto current_pos = ray.o + t * ray.d;
      auto sigma_t     = vec3f(eval_vpt_density(vsdf, current_pos));
      auto sigma_s     = vsdf.scatter * sigma_t;
      auto sigma_a     = sigma_t * (1.0 - vsdf.scatter);
      auto sigma_n     = vec3f(density_vol->max_voxel) - sigma_t;
      if (rand1f(rng) < mean(sigma_a) / majorant_density) {
	//printf("%f %f %f\n", sigma_a.x, sigma_a.y, sigma_a.z);
	return {path_length, sigma_a};
      } else if (rand1f(rng) - mean(sigma_n) / majorant_density) {
	d = sample_phasefunction(vsdf.anisotropy, -ray.d, rand2f(rng));
      }
      ray = {current_pos, d};
    }
    return {max_distance, vec3f{1}};
  }

  vec3f eval_vpt_transmittance(const vsdf& vsdf, float max_distance, rng_state& rng,
			       const ray3f& ray) {
    // Precompute values for Monte Carlo sampling on img::volume<float>
    auto object       = vsdf.object;
    auto density_vol  = object->density_vol;
    auto emission_vol = object->emission_vol;
    auto max_density  = density_vol->max_voxel * object->density_mult;
    auto imax_density = 1.0f / max_density;
    // Majorant of sigma_t
    auto majorant_density = max_density;
    auto path_length = 0.0f;
    auto tr          = 1.0f;

    while (true) {
      path_length -= log(1 - rand1f(rng)) / majorant_density;
      if (path_length >= max_distance)
	break;
      auto current_pos = ray.o + path_length * ray.d;
      auto density = eval_vpt_density(vsdf, current_pos) * imax_density;
      tr *= 1 - max((float)0, density);
    }
    return vec3f(tr);
  }

  static int sample_event(float pa, float ps, float pn, float rn) {
    // https://stackoverflow.com/a/26751752
    auto weights = vec3f{pa, ps, pn};
    auto numbers = vec3i{EVENT_ABSORB, EVENT_SCATTER, EVENT_NULL};
    float sum = 0.0f;
    for (int i = 0; i < 3; ++i) {
      sum += weights[i];
      if(rn < sum) {
	//printf("weights: pa: %f\tps: %f\tpn: %f\tsum: %i\n", pa, ps, pn, numbers[i]);
	return numbers[i];
      }
    }
    // Can reach this point only if |weights| < 1 (WRONG)
    //printf("I should not be here : %f\n", pa + ps + pn);
    return EVENT_NULL;
  }

  std::pair<float, vec3f> eval_unidirectional_spectral_mis(vsdf& vsdf, float max_distance,
							   rng_state& rng, const ray3f& ray) {
    auto object           = vsdf.object;
    auto density_vol      = object->density_vol;
    auto emission_vol     = object->emission_vol;
    auto max_density      = density_vol->max_voxel * object->density_mult;
    auto imax_density     = 1.0f / max_density;
    auto majorant_density = max_density;
    auto path_length      = 0.0f;
    auto f                = vec3f(1);
    auto p                = vec3f(1);
    auto cc               = clamp((int)(rand1f(rng) * 3), 0, 2);

    while (true) {
      path_length     -= log(1 - rand1f(rng)) / majorant_density;
      // Handle lengths bigger than max_distance
      auto current_pos = ray.o + path_length * ray.d;
      auto sigma_t     = vec3f(eval_vpt_density(vsdf, current_pos));
      auto sigma_s     = vsdf.scatter * sigma_t;
      auto sigma_a     = sigma_t * (1.0 - vsdf.scatter[cc]);
      auto sigma_n     = vec3f(density_vol->max_voxel) - sigma_t;
      // Sample event
      auto mu_e        = zero3f;
      auto e = sample_event(sigma_a[cc] * imax_density,
			    sigma_s[cc] * imax_density,
			    sigma_n[cc] * imax_density, rand1f(rng));
      switch(e) {
      case EVENT_NULL:
	mu_e = sigma_n;
	break;
      case EVENT_SCATTER:
	mu_e = sigma_s;
	break;
      case EVENT_ABSORB:
	mu_e = sigma_a;
	break;
      }
      f = f * eval_vpt_transmittance(vsdf, path_length, rng, ray) * mu_e;
      p = f;
      if (e == EVENT_ABSORB) {
	// TODO
      }
    }
    
    return {0.0, zero3f};    
  }

  std::pair<float, vec3f> delta_tracking(vsdf& vsdf, float max_distance, float rn,
						float eps, const ray3f& ray) {
    // Precompute values for Monte Carlo sampling on img::volume<float>
    auto object      = vsdf.object;
    auto density     = object->density_vol;
    auto emission    = object->emission_vol;
    auto max_density = density->max_voxel * object->density_mult;
    auto imax_density = 1.0f / max_density;

    auto weight = vec3f{1.0f};
    

    float sigma_t = 1.0f;
    // Sample distance in the volume
    float t = - log(1.0 - eps) * imax_density / sigma_t;
    // float t = - log(1.0 - eps) / 20.f;
    if (t >= max_distance) {
      // intersection out of volume bounds
      return {max_distance, weight};
    }
    auto vdensity = eval_vpt_density(vsdf, ray.o + t * ray.d);
    vsdf.density = vec3f{vdensity, vdensity, vdensity};    
    if (vdensity > rn) {
      // return transmittance
      //printf("vdensity : %f\tcorr_density : %f\n", vdensity, vdensity * imax_density);
      // Russian roulette based on albedo
      if (vsdf.scatter.x < rn) {
      	      return {t, zero3f};      
      }
      auto w_val = 1.0 - max(0.0f, vdensity * imax_density);
      return {t, weight * w_val};
    }
    return {t, weight};  
  }
  
  

  std::pair<float, vec3f> spectral_MIS(vsdf& vsdf, float max_distance, float rni,
				       float rn, float eps, const ray3f& ray, int& event) {
    auto path_contrib = vec3f(1);
    auto path_pdf     = vec3f(1);
    auto color_comp   = clamp((int)(rni * 3), 0, 2);
    auto density_vol  = vsdf.object->density_vol;
    auto imax_density   = 1.0f / (density_vol->max_voxel * vsdf.object->density_mult);
    auto t            = -log(1 - eps) * imax_density;
    if (t >= max_distance) t = max_distance;
    auto vdensity     = eval_vpt_density(vsdf, ray.o + t * ray.d);
    auto density      = vec3f{vdensity, vdensity, vdensity};
    // apply perling noise for albedo
    // auto albedo       = vec3f{perlin_noise(vec3f{ray.o.x, 0.0, 0.0}),
    // 			      perlin_noise(vec3f{0.0, ray.o.y, 0.0}),
    // 			      perlin_noise(vec3f{0.0, 0.0, ray.o.z})};
    //auto albedo = vec3f{perlin_noise(vec3f{ray.o})};
    //auto albedo = vec3f{0.6f};
    // We want to generate a random albedo, correlated to the scattering values of the JSON
    auto albedo = vsdf.scatter;
    // albedo.x = clamp(albedo.x + perlin_noise(ray.o + t*ray.d), 0.0, 1.0);
    // albedo.y = clamp(albedo.y + perlin_noise(ray.o + t*ray.d), 0.0, 1.0);
    //albedo.x = perlin_noise(ray.o + t * ray.d);
    vsdf.density      = density;

    auto sigma_t      = density;
    // if (sigma_t.x == 0.0f) {
    //   return {t, vec3f(1)};
    // }
    auto sigma_s      = albedo * sigma_t;
    auto sigma_a      = sigma_t * (1.0 - albedo[color_comp]);
    auto sigma_n      = vec3f(density_vol->max_voxel) - sigma_t;

    auto new_pos      = ray.o + t * ray.d; // new ray position

    auto e = sample_event(sigma_a[color_comp] * imax_density,
    			  sigma_s[color_comp] * imax_density,
    			  sigma_n[color_comp] * imax_density, rn);
    
    auto mu_e = zero3f;
    switch(e) {
    case EVENT_NULL:
      mu_e = sigma_n;
      break;
    case EVENT_SCATTER:
      mu_e = sigma_s;
      break;
    case EVENT_ABSORB:
      mu_e = sigma_a;
      break;
    }
    event = e;
    path_contrib *= (eval_transmittance(sigma_t, t));
    path_pdf      = path_contrib;

    return {t, path_contrib};
    
    // if (e == EVENT_ABSORB) {
    //   return {t, path_contrib};
    // } else if (e == EVENT_SCATTER) {
    //   return {t, path_contrib};
    //   // return {t, vec3f{1.0f}};
    // }
    // return {t, vec3f(1)};
  }

  

  void gen_volumetric(img::volume<float>* vol, const vec3i& size) {
    if(!vol) return;
    vol->resize(size);

    auto m = 0.0f;
    for (auto i = 0; i < size.x; i++) {
      for (auto j = 0; j < size.y; j++) {
        for (auto k = 0; k < size.z; k++) {
          auto p = vec3f{float(i)/float(size.x), float(j)/float(size.y), float(k)/float(size.z)};
          auto val = perlin_noise(p, size);
          
          (*vol)[{i,j,k}] = clamp(val, 0.0f, 1.0f);
          if(val > m) m = val;
        }
      }
    }
    vol->max_voxel = m;
  }

}
