/**
 * yovdbload.cpp
 */

#include <yocto/yocto_math.h>
#include <yocto/yocto_image.h>

#include <iostream>
#include <string>
#include <algorithm> // max_element

using namespace yocto::math;
namespace img = yocto::image;

void test_inv_object_transform(void) {
  auto p1 = vec3f{10, 10, 0};

  auto f1 = identity3x4f;
  f1.o = vec3f{10, 10, 0};

  auto p1_f1 = transform_point(inverse(f1), p1);
  std::cout << p1_f1.x << ", " << p1_f1.y << ", " << p1_f1.z << std::endl;
  return;
}

void test_volume(void) {
  auto ivol = img::volume<float>();
  std::string err_str;
  int ret = 0;
  ret = img::load_volume("../ovdb_dumps/smoke2.vol", ivol, err_str);
  if (!ret) {
    std::cout << err_str << std::endl;
    return;
  }
  std::cout << "Success, loaded volume." << std::endl;
  std::cout << "Volume properties:" << std::endl;
  std::cout << "Size : (" << ivol.extent.x << ", " << ivol.extent.y << ", " << ivol.extent.z << ")" << std::endl;
  std::cout << "True voxel array size : " << ivol.voxels.size() << std::endl;
  std::cout << "Density at voxel (113, 113, 113) : " << ivol[vec3i{113, 113, 113}] << std::endl;
  std::cout << "Density at voxel (0, 0, 0) : " << ivol[vec3i{0, 0, 0}] << std::endl;
  std::cout << "Max density value: " << *std::max_element(ivol.voxels.begin(), ivol.voxels.end()) << std::endl;
  
}

int main(int argc, char* argv[]) {
  //test_inv_object_transform();
  test_volume();
  return 0;
}
