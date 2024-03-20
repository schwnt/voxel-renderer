#include "voxel.h"
#include <optional>
#include <limits>
#include <vector>

namespace Voxel {

  voxel_t chunk_t::Test_Type_Local(v3i p) {

	assert((p.x >= 0 && p.x < 16) || (p.z >= 0 && p.z < 16));

	auto old_p = p;

	auto* chunk = this;
	if (p.x < 0) {
	  chunk = _adj[Direction2D::nx];
	  p.x += 16;
	}
	else if (p.x > 15) {
	  chunk = _adj[Direction2D::px];
	  p.x -= 16;
	}
	else if (p.z < 0) {
	  chunk = _adj[Direction2D::nz];
	  p.z += 16;
	}
	else if (p.z > 15) {
	  chunk = _adj[Direction2D::pz];
	  p.z -= 16;
	}

	// nothing here; it will not obstructs other blocks
	if (!chunk || p.y < 0 || p.y > 255) {
	  return (voxel_t)0;
	}
	return chunk->_data[p.x][p.y][p.z].type;
  }

  voxel_placement_t& chunk_t::Get_Local(const v3i& p) {
	assert(p.x >= 0 && p.x < 16);
	assert(p.y >= 0 && p.y < 256);
	assert(p.z >= 0 && p.z < 16);
	return _data[p.x][p.y][p.z];
  }

  void chunk_t::Put_Local(const v3i& p, voxel_t type, const v3i& placed_on) {
	assert(p.x >= 0 && p.x < 16);
	assert(p.y >= 0 && p.y < 256);
	assert(p.z >= 0 && p.z < 16);

	Get_Local(p) = voxel_placement_t{ type, Direction3D::to_id(placed_on) };
  }

  void chunk_t::SetNeighbor(v2i dir, chunk_t& chunk) {
	if (dir == v2i(0, 1)) {
	  _adj[0] = &chunk;
	  chunk._adj[2] = this;
	}
	else if (dir == v2i(1, 0)) {
	  _adj[1] = &chunk;
	  chunk._adj[3] = this;
	}
	else if (dir == v2i(0, -1)) {
	  _adj[2] = &chunk;
	  chunk._adj[0] = this;
	}
	else if (dir == v2i(-1, 0)) {
	  _adj[3] = &chunk;
	  chunk._adj[1] = this;
	}
	else assert(false);
  }

  void chunk_t::UnsetNeighbor() {
	if (_adj[0]) _adj[0]->_adj[2] = nullptr;
	if (_adj[1]) _adj[1]->_adj[3] = nullptr;
	if (_adj[2]) _adj[2]->_adj[0] = nullptr;
	if (_adj[3]) _adj[3]->_adj[1] = nullptr;
  }

  std::optional<ray_result_t> chunk_t::CastRayLocal(v3 origin, v3 direction, float max_distance, std::vector<voxel_t>& excludes) {
	direction = glm::normalize(direction);
	float distance = 0.0f;

	while (max_distance > 0) {
	  v3 fract = glm::fract(origin);
	  int axis = 0;
	  float axis_dist = std::numeric_limits<float>::max();

	  for (int i = 0; i < 3; ++i) {
		if (abs(direction[i]) < 0.001f) continue;

		float d = 0.0f;
		if (signbit(direction[i]) == signbit(fract[i]))
		  d = 1.0f - fract[i];
		else d = fract[i];

		if (d < axis_dist) {
		  axis = i;
		  axis_dist = d;
		}
	  }

	  float dist = abs(axis_dist / direction[axis]);

	  if (dist > max_distance) return std::nullopt;

	  v3i hit_voxel(origin);
	  hit_voxel[axis] += glm::sign((int)direction[axis]);

	  if (hit_voxel.x < 0 || hit_voxel.x >15 ||
		hit_voxel.y < 0 || hit_voxel.y >255 ||
		hit_voxel.z < 0 || hit_voxel.z >15) {
		distance += dist;
		max_distance -= dist;
		origin += direction * dist;
		continue;
	  }

	  auto blocktype = Get_Local(hit_voxel).type;
	  for (auto& excluded : excludes) {
		if (blocktype == excluded) {
		  distance += dist;
		  max_distance -= dist;
		  origin += direction * dist;
		  continue;
		}
	  }

	  v3 hit_position = origin + direction * dist;
	  distance += dist;
	  return ray_result_t{ hit_voxel, hit_position, distance, blocktype };
	}
  }

}