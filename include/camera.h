#pragma once
#include "commons.h"
#include "frustum.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/glm.hpp>

struct camera_t {
  camera_t() = default;

  camera_t(v3 pos) : _position(pos), _angles(0.0f),
	_fovy{ 0 }, _aspect{ 0 }, _near{ 0 }, _far{ 0 } {}

  camera_t(v3 pos, float fovy, float aspect, float near = 0.1f, float far = 400.0f)
	: _position(pos), _angles(0.0f),
	_fovy{ fovy }, _aspect{ aspect }, _near{ near }, _far{ far } {}

  mat4 Get_Transform_Matrix() const {
	mat4 translation = glm::translate(glm::identity<mat4>(), -_position);
	mat4 rotation = glm::rotate(glm::identity<mat4>(), _angles.x, v3(1.0f, 0, 0))
	  * glm::rotate(glm::identity<mat4>(), _angles.y, v3(0.0f, 1.0f, 0));
	return glm::perspective(glm::radians(_fovy), _aspect, _near, _far) * rotation * translation;
  }

  mat4 GetTransformNoPos() const {
	mat4 rotation = glm::rotate(glm::identity<mat4>(), _angles.x, v3(1.0f, 0, 0))
	  * glm::rotate(glm::identity<mat4>(), _angles.y, v3(0.0f, 1.0f, 0));
	return glm::perspective(glm::radians(_fovy), _aspect, _near, _far) * rotation;
  }

  v3 GetLook() const {
	//mat4 rotation = glm::rotate(glm::identity<mat4>(), _angles.x, v3(-1.0f, 0, 0))
	//  * glm::rotate(glm::identity<mat4>(), _angles.y, v3(0.0f, -1.0f, 0));
	mat4 rotation =
	  glm::rotate(glm::identity<mat4>(), _angles.y, v3(0.0f, -1.0f, 0))*
	   glm::rotate(glm::identity<mat4>(), _angles.x, v3(-1.0f, 0, 0));

	return rotation * v4(0, 0, -1.0f, 0.0f);
  }

  frustum_t GetFrustum() const {
	v3 look = GetLook();
	v3 right = glm::cross(look, v3(0, 1.0f, 0));
	v3 up = glm::cross(right, look);

	const float halfVSide = _far * tanf(_fovy * .5f);
	const float halfHSide = halfVSide * _aspect;
	const glm::vec3 frontMultFar = _far * look;

	return {
	.top = { _position, glm::cross(right, frontMultFar - up * halfVSide) },
	.bottom = { _position, glm::cross(frontMultFar + up * halfVSide, right) },
	.left = { _position, glm::cross(up, frontMultFar + right * halfHSide) },
	.right = { _position, glm::cross(frontMultFar - right * halfHSide, up) },
	.near = { _position + _near * look, look },
	.far = { _position + frontMultFar, -look }
	};
  }

  v3 _position;
  v3 _angles;
  float _fovy, _aspect, _near, _far;
};