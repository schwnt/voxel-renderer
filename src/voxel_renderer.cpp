#include "voxel_renderer.h"
#include "input.h"
#include <stb_image/stb_image.h>
#include <ranges>
#include <bitset>
#include <array>
#include <fstream>
#include <string>
#include <ranges>
#include <span>
#include <glm/gtx/vector_angle.hpp>
//#include <iostream>

voxel_renderer_t::voxel_renderer_t(GLFWwindow* window, Voxel::TextureLoader& texture_loader) : _window{ window }, _uniform_pos{} {
  GLuint vert_shader = CreateShader(RESOURCES_PATH"voxel.vert", GL_VERTEX_SHADER);
  GLuint frag_shader = CreateShader(RESOURCES_PATH"voxel.frag", GL_FRAGMENT_SHADER);

  _program = glCreateProgram();
  glAttachShader(_program, vert_shader);
  glAttachShader(_program, frag_shader);
  glLinkProgram(_program);

  glDeleteShader(vert_shader);
  glDeleteShader(frag_shader);

  ValidateProgram();

  glUseProgram(_program);

  _uniform_pos.transform = glGetUniformLocation(_program, "transform");
  _uniform_pos.texture = glGetUniformLocation(_program, "tex_array");
  _uniform_pos.skylight = glGetUniformLocation(_program, "skylight");
  _uniform_pos.colormap = glGetUniformLocation(_program, "colormap");

  glUseProgram(0);

  CreateTexture(texture_loader);
  CreateColorMap(texture_loader);

}

voxel_renderer_t::~voxel_renderer_t() {
  for (auto mesh : _meshs | std::views::values)
	glDeleteVertexArrays(1, &mesh.vao);
  _meshs.clear();
}

void voxel_renderer_t::Draw_All(camera_t& camera, const v3& skylight) {
  static timepoint_t start = chrono_clock::now();
  static int length = 0;
  static bool dist_dbg = false;
  static bool dbg_cull_face = true;
  static bool dgb_line = false;
  static bool dbg_frustum_reverse = false;
  static bool dbg_frustum_disable = true;
  if (Input::IsJustPressed("num 8")) {
	dist_dbg = !dist_dbg;
	length = 0;
  }
  if (Input::IsJustPressed("num 7")) {
	dbg_cull_face = !dbg_cull_face;
  }
  if (Input::IsJustPressed("num 9")) {
	dgb_line = !dgb_line;
  }
  if (Input::IsJustPressed("L")) {
	dbg_frustum_reverse = !dbg_frustum_reverse;
  }
  if (Input::IsJustPressed("K")) {
	dbg_frustum_disable = !dbg_frustum_reverse;
  }

  if (chrono_clock::now() - start > 1.5s) {
	length = ++length % 10;
	start = chrono_clock::now();
	//if (dist_dbg)
	 // std::cout << "length = " << length << "\n";
  }

  glUseProgram(_program);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D_ARRAY, _texture);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D_ARRAY, _color_map_texture);

  mat4 transform = camera.Get_Transform_Matrix();
  glUniformMatrix4fv(_uniform_pos.transform, 1, GL_FALSE, &transform[0][0]);
  glUniform3f(_uniform_pos.skylight, skylight.x, skylight.y, skylight.z);
  glUniform1i(_uniform_pos.texture, 0);
  glUniform1i(_uniform_pos.colormap, 1);

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  if (dbg_cull_face) glEnable(GL_CULL_FACE);
  else glDisable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glFrontFace(GL_CCW);

  frustum_t view_frustum = camera.GetFrustum();

  //for (mesh_t& mesh : _meshs | std::views::values) {
  for (auto& [p, mesh] : _meshs) {
	if (dist_dbg)
	  if (glm::max(glm::abs(p[0]), glm::abs(p[1])) / 16 != length) continue;

	v3 center(p[0] + 8, 78, p[1] + 8);
	bool frustum_cull = !view_frustum.TestAABB(center, v3(16, 500, 16));
	if (dbg_frustum_reverse)
	  frustum_cull = !frustum_cull;
	frustum_cull &= !dbg_frustum_disable;

	if (frustum_cull) {
	  continue;
	}

	glBindVertexArray(mesh.vao);
	if (dgb_line) {
	  glDrawElements(GL_LINES, mesh.elem_count, GL_UNSIGNED_INT, nullptr);
	}
	else {
	  glDrawElements(GL_TRIANGLES, mesh.elem_count, GL_UNSIGNED_INT, nullptr);
	}
  }

  glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
  glUseProgram(0);
}

void voxel_renderer_t::Update_Chunk(int x, int z, Voxel::chunk_t& chunk, chunk_biome_t& biome) {
  assert(x % 16 == 0 && z % 16 == 0);

  if (_meshs.contains({ x,z }))
	Free_Chunk(x, z);

  //std::vector<float> vertex_buffer;
  //std::vector<unsigned> index_buffer;
  //index_buffer.reserve(12582912);
  //vertex_buffer.reserve(18874368);
  //unsigned elem_count = 0;

  mesh_data_t mesh_data;
  InitMeshData(mesh_data);

  timepoint_t start = chrono_clock::now();

  for (int local_x = 0; local_x < 16; local_x++) {
	for (int local_y = 0; local_y < 256; local_y++) {
	  for (int local_z = 0; local_z < 16; local_z++) {
		v3i block_pos(local_x, local_y, local_z);
		auto block_type = (Block::block_t)chunk.Test_Type_Local(block_pos);
		if (block_type == Block::none) {
		  continue;
		}

		float& temperature = biome.temperature[local_x + local_z * 16];
		v3 pos_offset = v3(x + local_x, local_y, z + local_z);

		auto shape = GetShape(block_type);

		if (shape == BlockShape::cube) {
		  std::bitset<6> visible_faces;
		  visible_faces.reset();

		  for (auto direction_id : { 0,1,2,3,4,5 }) {
			v3i dir = Direction3D::to_v3i(direction_id);
			auto obstruct_blocktype = chunk.Test_Type_Local(block_pos + dir);
			bool seethrough = SeeThrough((Block::block_t)obstruct_blocktype);
			visible_faces.set(direction_id, seethrough);
		  }


		  AddCube(mesh_data, block_type, visible_faces, temperature, pos_offset);
		}
		else if (shape == BlockShape::cross) {
		  AddCross(mesh_data, block_type, temperature, pos_offset);
		}
	  }
	}
  }
  _meshs[v2i(x, z)] = CreateMesh(mesh_data);
}

void voxel_renderer_t::Free_Chunk(int x, int z) {
  if (!_meshs.contains({ x,z })) return;

  glDeleteBuffers(1, &_meshs[{x, z}].ibo);
  glDeleteVertexArrays(1, &_meshs[{x, z}].vao);

  _meshs.erase({ x,z });
}

GLuint voxel_renderer_t::CreateShader(const std::string& path, GLenum type) {
  std::ifstream file(path);

  if (!file.is_open()) {
	assert(false);
  }

  std::string src((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
  const char* src_str = src.c_str();
  file.close(); // Close the file

  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &src_str, NULL);
  glCompileShader(shader);

  GLint success;
  GLchar infoLog[512];
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
	glGetShaderInfoLog(shader, sizeof(infoLog), NULL, infoLog);
	assert(false);
  }
  return shader;
}

void voxel_renderer_t::ValidateProgram() {
  GLint info = 0;
  glGetProgramiv(_program, GL_LINK_STATUS, &info);
  if (info != GL_TRUE) {
	char* message = 0;
	int l = 0;
	glGetProgramiv(_program, GL_INFO_LOG_LENGTH, &l);
	message = new char[l];
	glGetProgramInfoLog(_program, l, &l, message);
	assert(false);
  }
  glValidateProgram(_program);
}

void voxel_renderer_t::CreateTexture(Voxel::TextureLoader& texture_loader) {
  for (int i = 0; i < Block::block_type_total; i++) {
	texture_loader.Load(Block::block_t(i));
  }

  const int w = 16, h = 16;
  const int depth = texture_loader.GetTextureCount();

  glGenTextures(1, &_texture);
  glBindTexture(GL_TEXTURE_2D_ARRAY, _texture);
  glTexStorage3D(GL_TEXTURE_2D_ARRAY, 5, GL_RGBA8, w, h, depth);

  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  //GLfloat anisotropy;
  //glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &anisotropy);
  //glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy);

  _tint_index.resize(Block::block_type_total);

  int current_index = 0;

  for (int i = 1; i < Block::block_type_total; i++) {
	auto blocktype = Block::block_t(i);

	auto blocktextures = texture_loader.Get(blocktype);

	_textures_id[blocktype].fill(-1);
	_tint_index[blocktype].fill(-1);

	for (auto& texture : blocktextures) {
	  assert(texture.texture.w == w && texture.texture.h == h);

	  glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, current_index, w, h, 1, GL_RGBA, GL_UNSIGNED_BYTE, texture.texture.data);

	  for (int i = 0; i < 6; i++) {
		if (texture.faces[i]) {
		  _textures_id[blocktype][i] = current_index;

		  _tint_index[blocktype][i] = texture.tint;
		}
	  }
	  current_index++;
	}
  }

  glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
  glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

void voxel_renderer_t::CreateColorMap(Voxel::TextureLoader& texture_loader) {
  std::array<std::string, 2> map_name = { "foliage.png" ,"grass.png" };
  //std::array<std::string, 2> map_name = { "foliage.png", "grass.png" };
  glGenTextures(1, &_color_map_texture);
  glBindTexture(GL_TEXTURE_2D_ARRAY, _color_map_texture);

  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  for (int i = 0; i < map_name.size(); i++) {
	auto& name = map_name[i];
	auto [w, h, channels, data] = texture_loader.LoadColorMap(name);
	if (i == 0) {
	  glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, w, h, map_name.size());
	}
	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, w, h, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
  }
  glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

void voxel_renderer_t::InitMeshData(mesh_data_t& data) {
  data.element_count = 0;
  data.postion.clear();
  data.postion.reserve(1572864);
  data.normal_id.clear();
  data.normal_id.reserve(1572864);
  data.texture_id.clear();
  data.texture_id.reserve(1572864);
  data.tint_index.clear();
  data.tint_index.reserve(1572864);
  data.color_pos.clear();
  data.color_pos.reserve(1572864);
  data.index_buffer.clear();
  data.index_buffer.reserve(1572864);
}

voxel_renderer_t::mesh_t voxel_renderer_t::CreateMesh(mesh_data_t& data) {
  assert(data.index_buffer.size() == data.element_count);
  assert(data.postion.size() * 6 == data.element_count * 4);
  assert(data.normal_id.size() * 6 == data.element_count * 4);
  assert(data.texture_id.size() * 6 == data.element_count * 4);
  assert(data.tint_index.size() * 6 == data.element_count * 4);
  assert(data.color_pos.size() * 6 == data.element_count * 4);

  GLuint vao, vbo, ibo;
  glCreateVertexArrays(1, &vao);
  glBindVertexArray(vao);
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  auto size_bytes = [](auto& v) ->size_t {return std::span(v).size_bytes(); };

  int pos_offset = 0;
  int texid_offset = pos_offset + size_bytes(data.postion);
  int normid_offset = texid_offset + size_bytes(data.texture_id);
  int tint_offset = normid_offset + size_bytes(data.normal_id);
  int color_offset = tint_offset + size_bytes(data.tint_index);

  glBufferData(GL_ARRAY_BUFFER,
	size_bytes(data.postion) + size_bytes(data.texture_id) + size_bytes(data.normal_id)
	+ size_bytes(data.tint_index) + size_bytes(data.color_pos),
	nullptr, GL_STATIC_DRAW);

  glBufferSubData(GL_ARRAY_BUFFER, pos_offset, size_bytes(data.postion), data.postion.data());
  glBufferSubData(GL_ARRAY_BUFFER, texid_offset, size_bytes(data.texture_id), data.texture_id.data());
  glBufferSubData(GL_ARRAY_BUFFER, normid_offset, size_bytes(data.normal_id), data.normal_id.data());
  glBufferSubData(GL_ARRAY_BUFFER, tint_offset, size_bytes(data.tint_index), data.tint_index.data());
  glBufferSubData(GL_ARRAY_BUFFER, color_offset, size_bytes(data.color_pos), data.color_pos.data());

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(v3), (void*)pos_offset);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(v2), (void*)color_offset);
  glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)texid_offset);
  glVertexAttribIPointer(3, 1, GL_UNSIGNED_INT, sizeof(unsigned), (void*)normid_offset);
  glVertexAttribIPointer(4, 1, GL_INT, sizeof(int), (void*)tint_offset);

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);
  glEnableVertexAttribArray(3);
  glEnableVertexAttribArray(4);

  glGenBuffers(1, &ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned) * data.index_buffer.size(), data.index_buffer.data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
  glDeleteBuffers(1, &vbo);

  return{ .vao = vao, .ibo = ibo, .elem_count = data.element_count };
}

float voxel_renderer_t::GetTexId(Block::block_t blocktype, int face) {
  return _textures_id[blocktype][face];
}

BlockShape::block_shape_t voxel_renderer_t::GetShape(Block::block_t blocktype) {
  if (blocktype == Block::weed0 || blocktype == Block::sapling_oak) return BlockShape::cross;
  return BlockShape::cube;
}

bool voxel_renderer_t::SeeThrough(Block::block_t blocktype) {
  if (blocktype == Block::none || GetShape(blocktype) == BlockShape::cross)
	return true;
  return false;
}

void voxel_renderer_t::AddCube(mesh_data_t& data, Block::block_t blocktype, std::bitset<6> visible_faces, float temperature, v3& pos_offset) {
  static const std::array<v3, 4> vert_pos[6] = {
	{
		v3{0.0f, 0.0f,  1.0f},
		v3{1.0f, 0.0f,  1.0f },
		v3{0.0f,  1.0f,  1.0f},
		v3{1.0f,  1.0f,  1.0f }
	  },
	{
	  v3{ 1.0f, 0.0f, 0.0f  },
	  v3{0.0f, 0.0f, 0.0f },
	  v3{ 1.0f,  1.0f, 0.0f  },
	  v3{0.0f,  1.0f, 0.0f }
	},
	{
		v3{0.0f, 0.0f, 0.0f },
		v3{0.0f, 0.0f,  1.0f },
		v3{0.0f,  1.0f, 0.0f  },
		v3{0.0f,  1.0f,  1.0f }
	},
	{
	  v3{1.0f, 0.0f,  1.0f},
	  v3{1.0f, 0.0f, 0.0f },
	  v3{1.0f,  1.0f,  1.0f },
	  v3{1.0f,  1.0f, 0.0f}
	},
	{
	  v3{0.0f,  1.0f,  1.0f },
	  v3{1.0f,  1.0f,  1.0f },
	  v3{0.0f,  1.0f, 0.0f },
	  v3{1.0f,  1.0f, 0.0f }
	},
	{
	  v3{0.0f, 0.0f, 0.0f } ,
	  v3{1.0f, 0.0f, 0.0f},
	  v3{0.0f, 0.0f,  1.0f} ,
	  v3{1.0f, 0.0f,  1.0f}
	}
  };

  if (!visible_faces.count()) return;
  if (blocktype == Block::none)
	return;

  data.element_count += visible_faces.count() * 6;

  for (int face : {0, 1, 2, 3, 4, 5}) {
	if (!visible_faces[face])
	  continue;

	const float texure_index = GetTexId(blocktype, face);

	assert(texure_index >= 0);

	for (int vert_n : {0, 1, 2, 3}) {
	  v3 pos = vert_pos[face][vert_n] + pos_offset;

	  data.postion.emplace_back(pos);
	  data.texture_id.emplace_back(texure_index);
	  data.normal_id.emplace_back(face);
	  data.tint_index.emplace_back(_tint_index[blocktype][face]);
	  data.color_pos.emplace_back(0.5f + 0.5f* temperature, pos.y / 256.0f);
	  //data.color_pos.emplace_back(-0.9f, 0.1f);
	}

	unsigned index_offset = data.index_buffer.empty() ? 0 : data.index_buffer.back() + 1;
	for (int i : { 0, 1, 2, 2, 1, 3 })
	  data.index_buffer.emplace_back(index_offset + i);
  }
}

void voxel_renderer_t::AddCross(mesh_data_t& data, Block::block_t blocktype, float temperature, v3& pos_offset) {
  /* TODO */
  return;
  static const std::array<v3, 4> cross_pos[4] = {
  {
	v3{0.0f, 0.0f,  0.0f},
	v3{1.0f, 0.0f,  1.0f },
	v3{0.0f,  1.0f,  0.0f},
	v3{1.0f,  1.0f,  1.0f }
  },
  {
	v3{1.0f, 0.0f,  0.0f},
	v3{0.0f, 1.0f,  1.0f },
	v3{1.0f,  1.0f,  0.0f},
	v3{0.0f,  1.0f,  1.0f }
  },

  {
	v3{1.0f, 0.0f,  1.0f },
	v3{0.0f, 0.0f,  0.0f},
	v3{1.0f,  1.0f,  1.0f },
	v3{0.0f,  1.0f,  0.0f}
  },
  {
	v3{0.0f, 1.0f,  1.0f },
	v3{1.0f, 0.0f,  0.0f},
	v3{1.0f,  1.0f,  0.0f},
	v3{0.0f,  1.0f,  1.0f }
  }
  };

  const float texure_index = GetTexId(blocktype, 0);

  data.element_count += 4 * 6;

  assert(texure_index >= 0);

  for (int face : {0, 1, 2, 3}) {
	for (int vert_n : {0, 1, 2, 3}) {
	  v3 pos = cross_pos[face][vert_n] + pos_offset;

	  data.postion.emplace_back(pos);
	  data.texture_id.emplace_back(texure_index);
	  data.normal_id.emplace_back(face);
	  data.tint_index.emplace_back(_tint_index[blocktype][face]);
	  data.color_pos.emplace_back(temperature, pos.y / 256.0f);
	}

	unsigned index_offset = data.index_buffer.empty() ? 0 : data.index_buffer.back() + 1;
	for (int i : { 0, 1, 2, 2, 1, 3 })
	  data.index_buffer.emplace_back(index_offset + i);
  }

}
