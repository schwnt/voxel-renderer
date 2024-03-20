#pragma once
#include "voxel.h"
#include "camera.h"
#include "api.h"
#include "biome.h"
#include "resource_loader.h"
#include <unordered_map>
#include <bitset>
#include <span>
#include <array>

class voxel_renderer_t {
public:
  voxel_renderer_t(GLFWwindow* window, Voxel::TextureLoader& texture_loader);
  ~voxel_renderer_t();

  void Draw_All(camera_t& camera, const v3& skylight);
  void Update_Chunk(int x, int z, Voxel::chunk_t& data, chunk_biome_t& biome);
  void Free_Chunk(int x, int z);

private:
  struct mesh_t {
	GLuint vao;
	GLuint ibo;
	unsigned elem_count;
	unsigned instance_count;
  };

  struct mesh_data_t {
	std::vector<v3> postion;
	std::vector<float> texture_id;
	std::vector<unsigned> normal_id;
	std::vector<unsigned> index_buffer;
	std::vector<int> tint_index;
	std::vector<v2> color_pos;
	unsigned element_count;

  };

  struct uniform_pos_t {
	GLint transform;
	GLint texture;
	GLint skylight;
	GLint colormap;
  };

  void CreateTexture(Voxel::TextureLoader& texture_loader);
  void CreateColorMap(Voxel::TextureLoader& texture_loader);
  GLuint CreateShader(const std::string& path, GLenum type);
  void ValidateProgram();
  void InitMeshData(mesh_data_t& data);
  void AddCube(mesh_data_t& data, Block::block_t blocktype, std::bitset<6> visible_faces, float temerature, v3& pos_offset);
  void AddCross(mesh_data_t& data, Block::block_t blocktype, float temerature, v3& pos_offset);
  mesh_t CreateMesh(mesh_data_t& data);
  float GetTexId(Block::block_t blocktype, int face);
  BlockShape::block_shape_t GetShape(Block::block_t blocktype);
  bool SeeThrough(Block::block_t blocktype);

  uniform_pos_t _uniform_pos;
  GLuint _program;
  GLuint _texture;
  GLuint _color_map_texture;
  std::unordered_map<v2i, mesh_t> _meshs;
  GLFWwindow* _window;
  std::unordered_map<Block::block_t, std::array<int, 6>> _textures_id;
  std::vector<std::array<int,6>> _tint_index;
};