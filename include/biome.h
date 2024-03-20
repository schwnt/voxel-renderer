#pragma once
#include "commons.h"
#include <vector>
#include <array>

struct chunk_biome_t {
  std::array<float, 256> temperature;
  float GetTemperature(int x, int z) { return 0.5f + temperature[x + 16 * z] * 0.5f; }
  v2 GetColorCoord(const v3i& p) {
	return v2(-0.9f, 0.1f);
  }
};
