#include "terrain.h"
#include <ranges>
#include <span>
#include <vector>
#include <fstream>
#include <nlohmann/json.hpp>


using json_t = nlohmann::json;

namespace Voxel {
  void terrain_t::GenerateChunk(chunk_t& chunk, int chunk_x, int chunk_z, chunk_biome_t& biome) {

	assert(chunk_x % 16 == 0 && chunk_z % 16 == 0);

	const int seed_offset[] = { 7891, 23459 ,56789, 97 };

	struct {
	  std::vector<float> continential, erosion, pv, plane, density;
	} noise;

	noise.continential.resize(16 * 16);

	if (_noise_param.continental.type == noise_param_t::simplex_type)
	  GenFromSimplex(noise.continential, chunk_x, chunk_z, _noise_param.continental, _seed + seed_offset[0]);
	else if (_noise_param.continental.type == noise_param_t::fractal_type)
	  GenFromFractal(noise.continential, chunk_x, chunk_z, _noise_param.continental, _seed + seed_offset[0]);

	noise.erosion.resize(16 * 16);

	if (_noise_param.erosion.type == noise_param_t::simplex_type)
	  GenFromSimplex(noise.erosion, chunk_x, chunk_z, _noise_param.erosion, _seed + seed_offset[1]);
	else if (_noise_param.erosion.type == noise_param_t::fractal_type)
	  GenFromFractal(noise.erosion, chunk_x, chunk_z, _noise_param.erosion, _seed + seed_offset[1]);

	noise.pv.resize(16 * 16);

	if (_noise_param.pv.type == noise_param_t::simplex_type)
	  GenFromSimplex(noise.pv, chunk_x, chunk_z, _noise_param.pv, _seed + seed_offset[2]);
	else if (_noise_param.pv.type == noise_param_t::fractal_type)
	  GenFromFractal(noise.pv, chunk_x, chunk_z, _noise_param.pv, _seed + seed_offset[2]);

	noise.plane.resize(16 * 16);

	if (_noise_param.plane.type == noise_param_t::simplex_type)
	  GenFromSimplex(noise.plane, chunk_x, chunk_z, _noise_param.plane, _seed + seed_offset[3]);
	else if (_noise_param.plane.type == noise_param_t::fractal_type)
	  GenFromFractal(noise.plane, chunk_x, chunk_z, _noise_param.plane, _seed + seed_offset[3]);

	for (int x = chunk_x; x < chunk_x + 16; x++) {
	  for (int z = chunk_z; z < chunk_z + 16; z++) {
		int local_x{ x - chunk_x }, local_z{ z - chunk_z };
		float plane = (0.5f - 0.5f * noise.plane[local_x + local_z * 16]);

		int elevation = ElevationMap(_elevation_map.continental, noise.continential[local_x + local_z * 16]);
		elevation += ElevationMap(_elevation_map.erosion, noise.erosion[local_x + local_z * 16]) * plane;
		elevation += ElevationMap(_elevation_map.pv, noise.pv[local_x + local_z * 16]) * plane;

		for (int y = 0; y < elevation - 1; y++) {
		  chunk.Put_Local({ local_x,y,local_z }, Block::stone, { 0,-1,0 });
		}
		chunk.Put_Local({ local_x,elevation - 1,local_z }, Block::grass, { 0,-1,0 });
	  }
	}
  }


  void terrain_t::GetElevationDBG(int x, int z, int& cont, int& ero, int& pv) {
	int chunk_x = Voxel::GetChunkPos({ x,0,z })[0];
	int chunk_z = Voxel::GetChunkPos({ x,0,z })[1];
	int local_x = x - chunk_x, local_z = z - chunk_z;

	const int seed_offset[] = { 7891, 23459 ,56789 };

	struct {
	  std::vector<float> continential, erosion, pv;
	} noise;

	noise.continential.resize(16 * 16);

	if (_noise_param.continental.type == noise_param_t::simplex_type)
	  GenFromSimplex(noise.continential, chunk_x, chunk_z, _noise_param.continental, _seed + seed_offset[0]);
	else if (_noise_param.continental.type == noise_param_t::fractal_type)
	  GenFromFractal(noise.continential, chunk_x, chunk_z, _noise_param.continental, _seed + seed_offset[0]);

	noise.erosion.resize(16 * 16);

	if (_noise_param.erosion.type == noise_param_t::simplex_type)
	  GenFromSimplex(noise.erosion, chunk_x, chunk_z, _noise_param.erosion, _seed + seed_offset[1]);
	else if (_noise_param.erosion.type == noise_param_t::fractal_type)
	  GenFromFractal(noise.erosion, chunk_x, chunk_z, _noise_param.erosion, _seed + seed_offset[1]);

	noise.pv.resize(16 * 16);

	if (_noise_param.pv.type == noise_param_t::simplex_type)
	  GenFromSimplex(noise.pv, chunk_x, chunk_z, _noise_param.pv, _seed + seed_offset[2]);
	else if (_noise_param.pv.type == noise_param_t::fractal_type)
	  GenFromFractal(noise.pv, chunk_x, chunk_z, _noise_param.pv, _seed + seed_offset[2]);

	cont = ElevationMap(_elevation_map.continental, noise.continential[local_x + 16 * local_z]);
	ero = ElevationMap(_elevation_map.erosion, noise.erosion[local_x + 16 * local_z]);
	pv = ElevationMap(_elevation_map.pv, noise.pv[local_x + 16 * local_z]);

	int h[] = { noise.continential[local_x + 16 * local_z],
	  noise.erosion[local_x + 16 * local_z],
	  noise.pv[local_x + 16 * local_z]
	};
  }

  int terrain_t::ElevationMap(const std::vector<v2>& map, float val) {
	assert(!map.empty());
	for (int i = 0; i < map.size(); i++)
	  if (val < map[i].x) {
		if (i == 0)  return map[0].y;

		v2 a = map[i - 1], b = map[i];
		//liner interpolation between a.y b.y
		return a.y + (b.y - a.y) * (val - a.x) / (b.x - a.x);
	  }

	return map.back().y;
  }

  float terrain_t::DensityCutoff(const std::vector<v2>& map, float y) {
	assert(!map.empty());
	return -1.0f;
	for (int i = 0; i < map.size(); i++) {
	  if (y < map[i].y) {
		if (i == 0)
		  return map[0].x;

		v2 a = map[i - 1], b = map[i];
		//liner interpolation between a.y b.y
		return a.x + (b.x - a.x) * (y - a.y) / (b.y - a.y);
	  }
	}
	return map.back().x;
  }

  void terrain_t::GenFromSimplex(std::vector<float>& out, int x, int z, noise_param_t& param, int seed) {
	_simplex->GenUniformGrid2D(out.data(), x, z, 16, 16, param.freq, seed);
  }

  void terrain_t::GenFromFractal(std::vector<float>& out, int x, int z, noise_param_t& param, int seed) {
	_fractal->SetOctaveCount(param.octave);
	_fractal->SetLacunarity(param.lacunarity);
	_fractal->SetGain(param.gain);
	_fractal->GenUniformGrid2D(out.data(), x, z, 16, 16, param.freq, seed);
  }

  void terrain_t::GenFromSimplex(std::vector<float>& out, int x, int y, int z, noise_param_t& param, int seed) {
	_simplex->GenUniformGrid2D(out.data(), x, z, 16, 16, param.freq, seed);
	_simplex->GenUniformGrid3D(out.data(), x, y, z, 16, 256 - y, 16, param.freq, seed);
  }

  void terrain_t::GenFromFractal(std::vector<float>& out, int x, int y, int z, noise_param_t& param, int seed) {
	_fractal->SetOctaveCount(param.octave);
	_fractal->SetLacunarity(param.lacunarity);
	_fractal->SetGain(param.gain);
	_fractal->GenUniformGrid3D(out.data(), x, y, z, 16, 256 - y, 16, param.freq, seed);
  }

  terrain_t::terrain_t() {
	_simplex = FastNoise::New<FastNoise::Simplex>();
	_fractal = FastNoise::New<FastNoise::FractalFBm>();
	_fractal->SetSource(_simplex);
	_seed = rand();

	std::fstream f(RESOURCES_PATH "terrain.json");
	assert(f);
	json_t p = json_t::parse(f);
	f.close();

	_noise_param.continental.Parse(p["noise"]["continental"]);
	_noise_param.erosion.Parse(p["noise"]["erosion"]);
	_noise_param.pv.Parse(p["noise"]["pv"]);
	_noise_param.plane.Parse(p["noise"]["plane"]);
	_noise_param.density.Parse(p["noise"]["density"]);

	for (auto& point : p["values"]["continental"]) {
	  float x = point["x"].get<float>();
	  int y = point["y"].get<int>();
	  _elevation_map.continental.emplace_back(x, y);
	}

	for (auto& point : p["values"]["erosion"]) {
	  float x = point["x"].get<float>();
	  int y = point["y"].get<int>();
	  _elevation_map.erosion.emplace_back(x, y);
	}

	for (auto& point : p["values"]["pv"]) {
	  float x = point["x"].get<float>();
	  int y = point["y"].get<int>();
	  _elevation_map.pv.emplace_back(x, y);
	}

	for (auto& point : p["values"]["density"]) {
	  float x = point["x"].get<float>();
	  int y = point["y"].get<float>();
	  _density_map.emplace_back(x, y);
	}
  }

  void noise_param_t::Parse(json_t& obj) {
	if (obj["type"] == "simplex") type = simplex_type;
	else if (obj["type"] == "fractal") type = fractal_type;
	octave = obj["octave"];
	freq = obj["freq"];
	gain = obj["gain"];
	lacunarity = obj["lacunarity"];
  }
}