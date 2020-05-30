//
// # Yocto/Extension: Tiny Yocto/GL extension
//
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
//

#ifndef _YOCTO_EXTENSION_H_
#define _YOCTO_EXTENSION_H_

// -----------------------------------------------------------------------------
// INCLUDES
// -----------------------------------------------------------------------------

#include <yocto/yocto_image.h>
#include <yocto/yocto_math.h>
#include <yocto_pathtrace/yocto_pathtrace.h>

#include <atomic>
#include <future>
#include <memory>

// -----------------------------------------------------------------------------
// ALIASES
// -----------------------------------------------------------------------------
namespace yocto::extension {

// Namespace aliases
namespace ext = yocto::extension;
namespace img = yocto::image;
namespace trace = pathtrace::ptr;


// Math defitions
using math::bbox3f;
using math::byte;
using math::frame3f;
using math::identity3x4f;
using math::ray3f;
using math::rng_state;
using math::vec2f;
using math::vec2i;
using math::vec3b;
using math::vec3f;
using math::vec3i;
using math::vec4f;
using math::vec4i;
using math::zero2f;
using math::zero3f;

using trace::object;

} 

// -----------------------------------------------------------------------------
// RENDERING DATA
// -----------------------------------------------------------------------------
namespace yocto::extension {

  // vsdf 
  struct vsdf {
    vec3f density    = {0, 0, 0};
    vec3f scatter    = {0, 0, 0};
    float anisotropy = 0;
    // heterogeneous volume properties
    bool  htvolume   = false;
    vec3f scale = {1.0, 1.0, 1.0};
    // new heterogeneous volume properties
    // object contains density, emission, frame, scale, offset etc
    const trace::object* object = nullptr;
  };

#define EVENT_NULL     0
#define EVENT_SCATTER  1
#define EVENT_ABSORB   2

}

// -----------------------------------------------------------------------------
// HIGH LEVEL API
// -----------------------------------------------------------------------------
namespace yocto::extension {

  // check if we have a vpt volume // vpt 
  bool has_vptvolume(const object* object);
  
  // check if the indices are inside the bounds // vpt
  bool check_bounds(vec3i vox_idx, vec3i bounds);

  // check for vpt density volume // vpt
  bool has_vpt_density(const object* object);

  // check for vpt emission volume // vpt
  bool has_vpt_emission(const object* object);

  // evaluate vpt density // vpt
  float eval_vpt_density(const vsdf& vsdf, const vec3f& uvw);

  // evaluate vpt emission // vpt
  float eval_vpt_emission(const vsdf& vsdf, const vec3f& uvw);
    
  // Delta tracking implementation based on PBRT Book (chap. Light Transport II: Volume Rendering)
  std::pair<float, float> delta_tracking(vsdf& vsdf, float max_distance, float rn,
						float eps, const ray3f& ray);
  // TODO: Add comment
  std::pair<float, vec3f> spectral_MIS(vsdf& vsdf, float max_distance, float rni,
				       float rn, float eps, const ray3f& ray, int& event);
  
  // takes as input a volume ptr and fills it with a perlin volumetric texture // vpt
  void gen_volumetric(img::volume<float>* vol, const vec3i& size);

}

#endif
