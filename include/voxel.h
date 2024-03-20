#pragma once
#include "defs.h"
#include "commons.h"
#include <optional>
#include <array>

namespace Voxel {
  using voxel_t = u8;

  constexpr u8 CHUNK_WIDTH = 16, CHUNK_HEIGHT = 255;

  struct voxel_placement_t {
	voxel_t type = 0;
	u8 placed_on = Direction3D::ny;
  };

  struct ray_result_t {
	v3i voxel_position;
	v3 hit_position;
	float distance;
	voxel_t blocktype;
  };

  class chunk_t {
  public:

	voxel_t Test_Type_Local(v3i p);
	voxel_placement_t& Get_Local(const v3i& p);
	void Put_Local(const v3i& p, voxel_t type, const v3i& placed_on);
	void SetNeighbor(v2i dir, chunk_t& chunk);
	void UnsetNeighbor();
	std::optional<ray_result_t> CastRayLocal(v3 origin, v3 direction, float max_distance, std::vector<voxel_t>& exclude);

	voxel_placement_t _data[16][256][16] = {};
	std::array<chunk_t*, 4> _adj = {};
	int _height[16][16] = {};
  };

  inline v2i GetChunkPos(const v3& v) {
	v2i p(v.x, v.z);
	if (p[0] < 0) p[0] -= 16;
	if (p[1] < 0) p[1] -= 16;
	return { p.x - p.x % 16, p.y - p.y % 16 };
  }
}
