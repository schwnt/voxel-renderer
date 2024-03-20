#include "frustum.h"

bool frustum_t::TestAABB(const v3& center, const v3& extents) {
  return left.IsOnForwardPlaneAABB(center, extents) && right.IsOnForwardPlaneAABB(center, extents) &&
	top.IsOnForwardPlaneAABB(center, extents) && bottom.IsOnForwardPlaneAABB(center, extents) &&
	near.IsOnForwardPlaneAABB(center, extents) && far.IsOnForwardPlaneAABB(center, extents);
}

bool plane_t::IsOnForwardPlaneAABB(const v3& center, const v3& extents) {
  const float r = extents.x * glm::abs(normal.x) +
	extents.y * glm::abs(normal.y) + extents.z * glm::abs(normal.z);

  return -r <= GetSignedDistanceToPlane(center);
}
