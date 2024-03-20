#pragma once
#include "commons.h"

struct plane_t {
  plane_t() = default;
  plane_t(const v3& p, const v3& norm)
	: normal(glm::normalize(norm)),
	distance(glm::dot(normal, p))
  {}

  float GetSignedDistanceToPlane(const v3& point) const {
	return glm::dot(normal, point) - distance;
  }

  bool IsOnForwardPlaneAABB(const v3& center, const v3& extents);

  v3 normal;
  float distance;
};

struct frustum_t {
  plane_t top, bottom, left, right, near, far;

  bool TestAABB(const v3& center, const v3& extend);
};
