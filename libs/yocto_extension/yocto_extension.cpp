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
    return eval_volume(*vol, uvl * scale, false, true, true) * object->density_mult;
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
    return eval_volume(*vol, uvl * scale, false, true, true) * inv_max;
  }

  std::pair<float, float> delta_tracking(vsdf& vsdf, float max_distance, float rn,
						float eps, const ray3f& ray) {
    // Precompute values for Monte Carlo sampling on img::volume<float>
    auto object      = vsdf.object;
    auto density     = object->density_vol;
    auto emission    = object->emission_vol;
    auto max_density = density->max_voxel * object->density_mult;
    auto imax_density = 1.0f / max_density;
    

    float sigma_t = 1.0f;
    // Sample distance in the volume
    float t = - log(1.0 - eps) * imax_density / sigma_t;
    // float t = - log(1.0 - eps) / 20.f;
    if (t >= max_distance) {
      // intersection out of volume bounds
      return {max_distance, 1.0f};
    }
    // auto local_ipoint = transform_point(inverse(vsdf.oframe), ray.o + t * ray.d);
    // float vdensity = eval_volume(*vsdf.ovol, local_ipoint * vsdf.scale, false, true, true) * 10;

    auto vdensity = eval_vpt_density(vsdf, ray.o + t * ray.d);
    vsdf.density = vec3f{vdensity, vdensity, vdensity};

    // if (vdensity == 0.0f) {
    //   return {t, 1.0f};
    // } else {
    //   return {t, 1.0 - vdensity * imax_density};
    // }
    
    if (vdensity > rn) {
      // return transmittance
      //printf("vdensity : %f\tcorr_density : %f\n", vdensity, vdensity * imax_density);
      // Russian roulette based on albedo
      if (vsdf.scatter.x < rn) {
	return {t, 0.0};      
      }
      return {t, 1.0 - max(0.0f, vdensity * imax_density)};
    }
    return {t, 1.0};  
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
