/**
 * yovdbload.cpp
 */

#include <yocto/yocto_commonio.h>
#include <yocto/yocto_trace.h>
#include <yocto/yocto_math.h>
#include <yocto/yocto_image.h>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

using namespace yocto::math;
namespace cli = yocto::commonio;
namespace trace = yocto::trace;
namespace img = yocto::image;

using namespace std::string_literals;

struct ovdb_voxel {
  int32_t x;
  int32_t y;
  int32_t z;
  float density;
};

inline bool ovdb_voxel_compare(const ovdb_voxel &a, const ovdb_voxel &b) {
  return (a.z == b.z) ? ((a.y == b.y) ? (a.x < b.x) : (a.y < b.y)) : a.z < b.z;
}

std::vector<ovdb_voxel> read_ovdbdump(const std::string &filename, trace::progress_callback progress_cb) {
  std::vector<ovdb_voxel> ovoxel_vect;
  // read the file
  // http://www.cplusplus.com/reference/istream/istream/read/
  std::ifstream ifile(filename, std::ios::in | std::ios::binary);
  if(ifile.is_open()) {
    // get length of file:
    ifile.seekg (0, ifile.end);
    std::size_t length = ifile.tellg();
    ifile.seekg (0, ifile.beg);

    // handle progress
    if (progress_cb) progress_cb("reading ovdb", 0, length);
    
    for (std::size_t i = 0; i < length; i += (sizeof(int32_t) * 3 + sizeof(float))) {
      int32_t cx, cy, cz;
      float cdensity;
      ifile.read(reinterpret_cast<char *>(&cx), sizeof(int32_t));
      ifile.read(reinterpret_cast<char *>(&cy), sizeof(int32_t));
      ifile.read(reinterpret_cast<char *>(&cz), sizeof(int32_t));
      ifile.read(reinterpret_cast<char *>(&cdensity), sizeof(float));
      ovdb_voxel cvoxel = {cx, cy, cz, cdensity};
      ovoxel_vect.push_back(cvoxel);
      if (progress_cb) progress_cb("reading ovdb", i, length);
    }
    if (progress_cb) progress_cb("reading ovdb", length, length);
  } else {
    std::cout << "Could not read the file." << std::endl;
  }
  
  return ovoxel_vect;
}

void find_max_bounds(const std::vector<ovdb_voxel> &ovdb_vect, int32_t &max_x, int32_t &max_y, int32_t &max_z) {
  max_x = 0;
  max_y = 0;
  max_z = 0;
  for (auto const& vox : ovdb_vect) {
    max_x = max(max_x, vox.x);
    max_y = max(max_y, vox.y);
    max_z = max(max_z, vox.z);
  }
  max_x += 1;
  max_y += 1;
  max_z += 1;
}

void sort_ovdb_vector(std::vector<ovdb_voxel> &ovdb_vect) {
  std::sort(ovdb_vect.begin(), ovdb_vect.end(), ovdb_voxel_compare);
}

void expand_voxel_vect(std::vector<float> &ovect, const std::vector<ovdb_voxel> &ivect,
		       int32_t max_x, int32_t max_y, int32_t max_z, trace::progress_callback progress_cb) {
  if(progress_cb) progress_cb("expand voxel data", 0, ivect.size());
  std::size_t counter = 0;
  for (auto const &vox : ivect) {
    ovect[vox.z * max_y * max_x + vox.y * max_x + vox.x] = vox.density;
    counter++;
    if(progress_cb) progress_cb("expand voxel data", counter, ivect.size());
  }
}

int main(int argc, const char* argv[]) {
  auto ovdbfilename = "in1.bin"s;
  auto ofilename = "out.vol"s;

  // parse command line
  auto cli = cli::make_cli("yovdbload", "Bridge from ovdb active dump to yocto volume");
  add_option(cli, "input", ovdbfilename, "ovdb active voxel dump", true);
  add_option(cli, "--output-file,-o", ofilename, "Output filename");
  parse_cli(cli, argc, argv);

  std::vector<ovdb_voxel> idata = read_ovdbdump(ovdbfilename, cli::print_progress);
  std::cout << "Loaded " << idata.size() << " active voxels." << std::endl;
  std::cout << "Sorting loaded data...";
  sort_ovdb_vector(idata);
  std::cout << "Done." << std::endl;
  int32_t mx, my, mz;
  find_max_bounds(idata, mx, my, mz);
  std::cout << "BB bounds: (" << mx << ", " << my << ", " << mz << ")" << std::endl;
  std::vector<float> odata(mx * my * mz, 0.0f);
  expand_voxel_vect(odata, idata, mx, my, mz, cli::print_progress);

  img::volume<float> ovol(vec3i{mx, my, mz}, odata.data());
  std::string err_str;
  if (!img::save_volume(ofilename, ovol, err_str)) {
    std::cout << err_str << std::endl;
  }
  return 0;
} 
