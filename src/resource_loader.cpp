#include "resource_loader.h"
#include <fstream>
#include "stb_image/stb_image.h"
#include<iostream>

/*
	none, grass, dirt, stone, cobblestone, gravel, sand,
	oak, plank_oak, leaf_oak, bedrock, brick, glass, tnt, pumpkin, jacklantern,
	tourch, ore_iron, ore_gold, ore_diamond, ore_redstone, ore_coal,
	mushroom_red, mushroom_brown, web, lava, water,
	weed0, weed1, weed2, weed3, rose, yellowflower, sapling_oak,
	wheat0, wheat1, wheat2, wheat3, wheat4, wheat5, wheat6, wheat7, wheat8,
	farmland, farmland_wet, snow, ice, wool,
	crafting_table, furnace, furnace_lit, rail, rail_turn, block_type_total*/

namespace Block {
  static const std::unordered_map<block_t, std::string> model_name {
	{grass, "block/grass_normal"},
	{ dirt, "block/dirt" },
	{ stone, "block/stone" },
	{ cobblestone, "block/cobblestone" },
	{ gravel, "block/gravel" },
	{ sand, "block/sand" },
	{ oak, "block/oak_log" },
	{ leaf_oak, "block/oak_leaves" },
	{ bedrock, "block/bedrock" },
	{ glass, "block/glass" },
	{ tnt, "block/tnt" },
	{ pumpkin, "block/pumpkin" },
	{ jacklantern, "block/lit_pumpkin" },
	{ crafting_table, "block/crafting_table" },
	{ snow, "block/grass_snowed" },
	{ ice, "block/ice" },
	{ furnace, "block/furnace" },
	{ furnace_lit, "block/lit_furnace" },
	{ ice, "block/ice" },
	{ weed0, "block/double_grass_bottom" }
  };
}

namespace Voxel {
  TextureLoader::TextureLoader() {
	//path_t asset_path(RESOURCES_PATH "Plastic Texture Pack by CrazySlimeYT");
	path_t asset_path(RESOURCES_PATH "Template Resource Pack [1.8.X]");
	//path_t asset_path(RESOURCES_PATH "New_Wayukian_Java_1.20");
	asset_path.make_preferred();

	_models = ModelLoader(asset_path / "assets" / "minecraft" / "models");
	_dir = asset_path / "assets" / "minecraft" / "textures";
	_texture_count = 0;
  }

  TextureLoader::~TextureLoader() {
	for (auto& [k, _] : _textures)
	  Free(k);
	for (auto& [k, _] : _colormap)
	  FreeColorMap(k);
  }

  void TextureLoader::Load(Block::block_t blocktype) {
	if (blocktype == Block::none) return;

	auto& model = _models->Get(GetModelRL(blocktype));

	auto& faces_obj = model["elements"][0]["faces"];
	static const std::vector<std::string> face_names = { "south", "north", "west", "east", "up", "down" };

	std::unordered_map<std::string, std::bitset<6>> texture_map;
	std::unordered_map<std::string, int> tints;

	for (int i = 0; i < 6; i++) {
	  const auto& face_name = face_names[i];

	  /*hack for cross blocks*/
	  if (!faces_obj.contains(face_name)) {
		auto& tex_rl = model["textures"].front();
		//std::cout << "\n" << std::setw(2) << model << "\n";
		//std::cout << "\n" << std::setw(2) << tex_rl << "\n";
		std::string aa = tex_rl;
		texture_map[tex_rl].set(i);
		tints[tex_rl] = -1;
		if (faces_obj[face_name].contains("tintindex")) {
		  tints[tex_rl] = faces_obj[face_name]["tintindex"].get<int>();
		}
		continue;
	  }

	  auto& tex_rl = faces_obj[face_name]["texture"];

	  texture_map[tex_rl].set(i);
	  tints[tex_rl] = -1;

	  if (faces_obj[face_name].contains("tintindex")) {
		tints[tex_rl] = faces_obj[face_name]["tintindex"].get<int>();
	  }

	}

	for (auto& [tex_rl, visible_faces] : texture_map) {
	  path_t tex_path = _dir / tex_rl;
	  tex_path.replace_extension("png").make_preferred();

	  assert(fs::exists(tex_path));

	  int w, h, channels;
	  stbi_set_flip_vertically_on_load(true);

	  unsigned char* data = stbi_load(tex_path.string().c_str(), &w, &h, &channels, 4);

	  assert(data);

	  TextureInfo info{ w,h,channels, data };
	  int tintindex = tints[tex_rl];

	  _textures[blocktype].emplace_back(info, visible_faces, tintindex);
	  _texture_count++;
	}
  }


  std::vector<BlockTexture> TextureLoader::Get(Block::block_t blocktype) const {
	assert(_textures.contains(blocktype));

	return _textures.at(blocktype);
  }

  int TextureLoader::GetTextureCount() const {

	return _texture_count;
  }

  std::string TextureLoader::GetModelRL(Block::block_t blocktype) {
	if (Block::model_name.contains(blocktype))
	  return Block::model_name.at(blocktype);

	return "block/crafting_table";
  }

  void TextureLoader::Free(Block::block_t blocktype) {
	if (!_textures.contains(blocktype)) return;

	for (auto& block_texture : _textures[blocktype])
	  stbi_image_free(block_texture.texture.data);

	_texture_count -= _textures[blocktype].size();

	_textures.erase(blocktype);
  }

  TextureInfo TextureLoader::LoadColorMap(const std::string& name) {
	if (_colormap.contains(name))
	  return _colormap[name];

	path_t path = _dir / "colormap" / name;
	path.make_preferred();
	assert(fs::exists(path));
	int w, h, channels;

	stbi_set_flip_vertically_on_load(true);

	unsigned char* data = stbi_load(path.string().c_str(), &w, &h, &channels, 4);
	assert(data);

	_colormap[name] = { w,h,channels, data };
	return _colormap[name];
  }

  void TextureLoader::FreeColorMap(const std::string& name) {
	if (_colormap.contains(name)) {
	  stbi_image_free(_colormap[name].data);
	  _colormap.erase(name);
	}
  }

}

ModelLoader::ModelLoader(const path_t& root) {
  _root = root;

}

ModelLoader::json_t& ModelLoader::Get(const std::string& rel_path) {
  /* create path to .json model file */
  path_t path = _root / rel_path;

  if (!path.has_extension())
	path.replace_extension("json");

  path.make_preferred();

  assert(fs::exists(path));
  assert(path.extension().string() == ".json");

  /* return cached object */
  if (_cache.contains(path))
	return _cache[path];

  /* load from file */
  std::ifstream f(path.c_str());
  _cache.emplace(path, json_t::parse(f));
  json_t& obj = _cache[path];
  f.close();

  /* finalize object by Block models rules
  https://minecraft.fandom.com/wiki/Tutorials/Models
  */
  if (obj.contains("parent"))
	Inherit(obj, Get(obj["parent"]));

  if (obj.contains("textures")) {
	auto& textures = obj["textures"];

	//solve referrences within "textures"
	for (auto& [k, v] : textures.items()) {
	  auto var = GetVariable(v);
	  if (!var) continue;

	  textures[k] = GetReferencedVal(textures, *var);
	}

	//solve referrences whole file
	SetTextureVariables(obj, textures);
  }

  return obj;
}

void ModelLoader::Inherit(json_t& child, const json_t& parent) {
  std::optional<json_t> new_elements;
  if (child.contains("elements"))
	new_elements = child["elements"];

  json_t new_child = parent;
  new_child.merge_patch(child);

  if (new_elements)
	new_child["elements"] = *new_elements;

  child = new_child;
}

void ModelLoader::SetTextureVariables(json_t& obj, const json_t& variables) {
  if (obj.is_array()) {
	for (auto& a : obj) {
	  if (a.is_object() || a.is_array())
		SetTextureVariables(a, variables);
	}
	return;
  }

  assert(obj.is_object());

  for (auto& [k, v] : obj.items()) {
	if (k == "texture") {
	  auto var = GetVariable(v);

	  //assign value
	  if (var && variables.contains(*var))
		v = variables[*var];
	}
	if (v.is_object() || v.is_array())
	  SetTextureVariables(v, variables);
  }
}

/* return value obtained through referrence chain within OBJ */
std::string ModelLoader::GetReferencedVal(json_t& obj, std::string_view key) {
  //return key as a variable name
  if (!obj.contains(key))
	return "#" + std::string(key);

  auto& v = obj[key];
  auto var_name = GetVariable(v);

  //v is not a variable
  if (!var_name) return v;

  return GetReferencedVal(obj, *var_name);
}

/* return nullopt if val is value, aka not a variable name*/
std::optional<std::string> ModelLoader::GetVariable(std::string val) {
  if (val[0] == '#') return val.substr(1);
  return std::nullopt;
}
