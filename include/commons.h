#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/hash.hpp>
#include <chrono>
#include <filesystem>

namespace fs = std::filesystem;
using path_t = std::filesystem::path;

using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;
using u64 = unsigned long long;
using i8 = char;
using i16 = short;
using i32 = int;
using i64 = long long;
//using f32 = float;
//using f64 = double;
using v2 = glm::vec2;
using v3 = glm::vec3;
using v4 = glm::vec4;
using v2i = glm::vec<2, int>;
using v3i = glm::vec<3, int>;
using v4i = glm::vec<4, int>;
using v2d = glm::vec<2, double>;
using v3d = glm::vec<3, double>;
using v4d = glm::vec<4, double>;
using v2u = glm::vec<2, unsigned int>;
using v3u = glm::vec<3, unsigned int>;
using v4u = glm::vec<4, unsigned int>;
using v3_u8 = glm::vec<3, unsigned char>;
using v4_u8 = glm::vec<4, unsigned char>;
using v2_u8 = glm::vec<2, unsigned char>;
using v3_i8 = glm::vec<3, char>;
using v4_i8 = glm::vec<4, char>;
using v2_i8 = glm::vec<2, char>;
using mat4 = glm::mat4;

struct transform_t {
  v3 position;
  glm::quat rotation;

  mat4 Get_Matrix() {
	return glm::translate(glm::toMat4(rotation), position);
  }
};

using namespace std::literals::chrono_literals;

using chrono_clock = std::chrono::high_resolution_clock;
using timepoint_t = chrono_clock::time_point;
using clock_duration_t = chrono_clock::duration;
using fract_duration_t = std::chrono::duration<double>;
using std::chrono::nanoseconds;
using std::chrono::microseconds;
using std::chrono::milliseconds;
using std::chrono::seconds;
using std::chrono::minutes;
using std::chrono::hours;

namespace Direction2D {
  enum id_t : u8 {
	pz, px, nz, nx, direction_2d_total
  };

  inline v3i to_v3i(u8 dir, int y = 0) {
	assert(dir < direction_2d_total);
	const u8 x[] = { 0,1,0,-1 }, z[] = { 1,0,-1,0 };
	return { x[dir], y, z[dir] };
  }
  inline v2i to_v2i(u8 dir) {
	assert(dir < direction_2d_total);
	const u8 x[] = { 0,1,0,-1 }, z[] = { 1,0,-1,0 };
	return { x[dir], z[dir] };
  }
  inline u8 to_id(int x, int z) {
	assert(abs(x + z) == 1 && x * z == 0);
	switch (x) {
	case -1: return nx;
	case 1: return px;
	}
	switch (z) {
	case -1: return nz;
	case 1: return pz;
	}
	return 0;
  }
}

namespace Direction3D {
  enum id_t {
	pz, nz, nx, px, py, ny, direction_3d_total
  };
  inline v3i to_v3i(int dir) {
	//assert(dir < direction_3d_total);
	constexpr int x[] = { 0,0,-1,1,0,0 }, z[] = { 1,-1,0,0,0,0 }, y[] = { 0,0,0,0,1,-1 };
	return v3i(x[dir], y[dir], z[dir]);
  }
  inline u8 to_id(const v3i& v) {
	//assert(abs(v.x + v.y + v.z) == 1 && v.x * v.y * v.z == 0);
	switch (v.x) {
	case -1: return nx;
	case 1: return px;
	}
	switch (v.y) {
	case -1: return ny;
	case 1: return py;
	}
	switch (v.z) {
	case -1: return nz;
	case 1: return pz;
	}
  }
}