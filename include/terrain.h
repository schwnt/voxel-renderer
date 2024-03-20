#pragma once
#include "voxel.h"
#include "biome.h"
#include <FastNoise/FastNoise.h>
#include <vector>
#include "commons.h"
#include <nlohmann/json.hpp>

using json_t = nlohmann::json;

namespace Voxel {

  struct terrain_layer_t {
	int blocktype;
	int size;
	int size_offset;
  };

  struct noise_param_t {
	enum {
	  simplex_type, fractal_type
	} type = simplex_type;
	int octave = 2;
	float freq = 0.005f, gain = 0.21f, lacunarity = 2.5f;

	void Parse(json_t& obj);
  };

  class terrain_t {
  public:
	terrain_t();
	std::vector<terrain_layer_t> _layers;
	int _seed;
	void GenerateChunk(chunk_t& chunk, int chunk_x, int chunk_z, chunk_biome_t& biome);
	void GetElevationDBG(int x, int z, int& cont, int& ero, int& pv);
  private:
	int ElevationMap(const std::vector<v2>& map, float val);
	float DensityCutoff(const std::vector<v2>& map, float y);

	void GenFromSimplex(std::vector<float>& out, int x, int z, noise_param_t& param, int seed);
	void GenFromFractal(std::vector<float>& out, int x, int z, noise_param_t& param, int seed);
	void GenFromSimplex(std::vector<float>& out, int x, int y, int z, noise_param_t& param, int seed);
	void GenFromFractal(std::vector<float>& out, int x, int y, int z, noise_param_t& param, int seed);

	struct {
	  std::vector<v2> continental, erosion, pv;
	} _elevation_map;

	std::vector<v2> _density_map;

	struct {
	  noise_param_t continental, erosion, pv, plane, density;
	} _noise_param;

	FastNoise::SmartNode<FastNoise::Simplex> _simplex;
	FastNoise::SmartNode<FastNoise::FractalFBm> _fractal;
  };

}
