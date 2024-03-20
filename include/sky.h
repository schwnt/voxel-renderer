#pragma once
#include "api.h"
#include "commons.h"
#include "camera.h"
#include <vector>
#include <unordered_map>


class sky_renderer_t {
public:
  sky_renderer_t(const std::vector<path_t>& image_paths, const std::vector<std::string>& names);
  ~sky_renderer_t();
  void DrawMix(const camera_t& camera, const std::vector<std::string>& images, const std::vector<float>& weights);
private:
  struct uniform_pos_t {
	GLint transform, images_factors;
	GLint texture[4];
  };

  void CreateTexture(const std::vector<path_t>& image_paths, const std::vector<std::string>& names);
  void CreateVao();
  void CreateProgram();
  GLuint CreateShader(const std::string& path, GLenum type);
  void ValidateProgram(GLuint program);

  std::unordered_map<std::string, GLuint> _textures;
  uniform_pos_t _uniform_pos;
  GLuint _vao, _program, _fog_program;
};