#include "sky.h"
#include "input.h"
#include<ranges>
#include <glm/gtx/component_wise.hpp>
#include <fstream>
#include <stb_image/stb_image.h>
#include <ranges>
#include <array>
#include <vector>
#include <iostream>

void sky_renderer_t::DrawMix(const camera_t& camera, const std::vector<std::string>& images, const std::vector<float>& weights) {
  assert(!images.empty() && !weights.empty());

  glBindVertexArray(_vao);
  glUseProgram(_program);
  glDepthFunc(GL_LEQUAL);
  glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

  float factors[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

  int texture_n = 0;
  for (int i = 0; i < images.size() && texture_n <= 4; i++) {
	if (!_textures.contains(images[i]))
	  continue;
	if (weights[i] > 0.0f) {
	  factors[i] = weights[i];

	  glUniform1i(_uniform_pos.texture[texture_n], texture_n);
	  glActiveTexture(GL_TEXTURE0 + texture_n);
	  glBindTexture(GL_TEXTURE_CUBE_MAP, _textures[images[i]]);

	  texture_n++;
	}
  }


  mat4 transform = camera.GetTransformNoPos();

  glUniform4fv(_uniform_pos.images_factors, 1, &factors[0]);
  glUniformMatrix4fv(_uniform_pos.transform, 1, GL_FALSE, &transform[0][0]);

  glDrawArrays(GL_TRIANGLES, 0, 36);

  glUseProgram(0);
  glBindVertexArray(0);
}

sky_renderer_t::sky_renderer_t(const std::vector<path_t>& image_paths, const std::vector<std::string>& names) {
  CreateTexture(image_paths, names);
  CreateVao();
  CreateProgram();
}

sky_renderer_t::~sky_renderer_t() {
  for (auto [_, texture] : _textures)
	glDeleteTextures(1, &texture);

  glDeleteProgram(_program);
  glDeleteVertexArrays(1, &_vao);
}

void sky_renderer_t::CreateTexture(const std::vector<path_t>& image_paths, const std::vector<std::string>& names) {
  assert(image_paths.size() <= names.size());

  int image_count = image_paths.size();

  for (int n_image = 0; n_image < image_count; n_image++) {
	if (_textures.count(names[n_image]))
	  _textures.erase(names[n_image]);

	GLuint& texture = _textures[names[n_image]];

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

	const std::array<int, 6> face_target {
	  GL_TEXTURE_CUBE_MAP_NEGATIVE_X, GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_POSITIVE_X,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
	};
	std::array<std::vector<unsigned char>, 6> image_data{};

	stbi_set_flip_vertically_on_load(true);

	assert(fs::exists(image_paths[n_image]));

	int w, h, channels;
	auto data = stbi_load(image_paths[n_image].string().c_str(), &w, &h, &channels, 4);
	assert(w / 3 == h / 2);

	int face_image_w = w / 3, face_image_h = h / 2;
	for (int y = h - 1; y >= 0; y--) {
	  for (int x = 0; x < w; x++) {
		int n_face = x / face_image_w + 3 * (y / face_image_h);

		//rgb data
		image_data[n_face].push_back(data[4 * (x + y * w)]);
		image_data[n_face].push_back(data[4 * (x + y * w) + 1]);
		image_data[n_face].push_back(data[4 * (x + y * w) + 2]);
		image_data[n_face].push_back(data[4 * (x + y * w) + 3]);
	  }
	}
	stbi_image_free(data);

	for (int n_face = 0; n_face < 6; n_face++) {
	  glTexImage2D(face_target[n_face], 0, GL_RGBA, w / 3, h / 2, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, image_data[n_face].data());
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
  }

}

void sky_renderer_t::CreateVao() {
  float skybox_verticies[] = {
	-1.0f,  1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	-1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f
  };
  glGenVertexArrays(1, &_vao);
  glBindVertexArray(_vao);

  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  glBufferData(GL_ARRAY_BUFFER, sizeof(skybox_verticies), skybox_verticies, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
  glEnableVertexAttribArray(0);

  glBindVertexArray(0);
  glDeleteBuffers(1, &vbo);
}

void sky_renderer_t::CreateProgram() {
  /*skybox */
  GLuint vert_shader = CreateShader(RESOURCES_PATH "sky.vert", GL_VERTEX_SHADER);
  GLuint frag_shader = CreateShader(RESOURCES_PATH "sky.frag", GL_FRAGMENT_SHADER);

  _program = glCreateProgram();
  glAttachShader(_program, vert_shader);
  glAttachShader(_program, frag_shader);
  glLinkProgram(_program);

  glDeleteShader(vert_shader);
  glDeleteShader(frag_shader);

  ValidateProgram(_program);


  glUseProgram(_program);

  _uniform_pos.transform = glGetUniformLocation(_program, "transform");
  _uniform_pos.texture[0] = glGetUniformLocation(_program, "skybox0");
  _uniform_pos.texture[1] = glGetUniformLocation(_program, "skybox1");
  _uniform_pos.texture[2] = glGetUniformLocation(_program, "skybox2");
  _uniform_pos.texture[3] = glGetUniformLocation(_program, "skybox3");
  _uniform_pos.images_factors = glGetUniformLocation(_program, "image_factors");

  ///*fog */
  //vert_shader = CreateShader(RESOURCES_PATH "fog.vert", GL_VERTEX_SHADER);
  //frag_shader = CreateShader(RESOURCES_PATH "fog.frag", GL_FRAGMENT_SHADER);

  //_fog_program = glCreateProgram();
  //glAttachShader(_fog_program, vert_shader);
  //glAttachShader(_fog_program, frag_shader);
  //glLinkProgram(_fog_program);

  //glDeleteShader(vert_shader);
  //glDeleteShader(frag_shader);

  //ValidateProgram(_fog_program);

  //glUseProgram(_fog_program);

  //_uniform_pos.transform = glGetUniformLocation(_program, "transform");
  //glUseProgram(0);
}

void sky_renderer_t::ValidateProgram(GLuint program) {
  GLint info = 0;
  glGetProgramiv(program, GL_LINK_STATUS, &info);
  if (info != GL_TRUE) {
	char* message = 0;
	int l = 0;
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &l);
	message = new char[l];
	glGetProgramInfoLog(program, l, &l, message);
	assert(false);
  }
  glValidateProgram(program);

}

GLuint sky_renderer_t::CreateShader(const std::string& path, GLenum type) {
  std::ifstream f(path);

  assert(f);

  std::string src((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
  const char* src_str = src.c_str();
  f.close(); // Close the file

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
