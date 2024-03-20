#pragma once
#include "defs.h"
#include "commons.h"
#include "nlohmann/json.hpp"
#include <vector>
#include <bitset>
#include <unordered_map>
#include <string>
#include <string_view>
#include <array>

class ModelLoader {
public:
  using json_t = nlohmann::json;

  ModelLoader(const path_t& root);
  json_t& Get(const std::string& rel_path);

  path_t _root;

private:
  void Inherit(json_t& child, const json_t& parent);
  void SetTextureVariables(json_t& obj, const json_t& variables);
  std::string GetReferencedVal(json_t& obj, std::string_view key);
  std::optional<std::string> GetVariable(std::string val);

  std::unordered_map<path_t, json_t> _cache;
};

struct TextureInfo {
  int w, h, channels;
  unsigned char* data;
};

struct BlockTexture {
  TextureInfo texture;
  std::bitset<6> faces;
  int tint;
};

namespace Voxel {
  class TextureLoader {
  public:
	TextureLoader();
	~TextureLoader();
	void Load(Block::block_t blocktype);
	void Free(Block::block_t blocktype);

	TextureInfo LoadColorMap(const std::string& name);
	void FreeColorMap(const std::string& name);

	std::vector<BlockTexture> Get(Block::block_t blocktype) const;
	int GetTextureCount() const;

  private:
	std::string GetModelRL(Block::block_t);

	std::optional<ModelLoader> _models;
	std::unordered_map<Block::block_t, std::vector<BlockTexture>> _textures;
	std::unordered_map<std::string, TextureInfo> _colormap;
	path_t _dir;
	int _texture_count;
  };

}