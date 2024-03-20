#include "player.h"
#include "input.h"
#include <iostream>

void player_t::Process(float delta, bool input_read) {
  if (input_read) {
	v3 move_dir(0.0f);
	v3 forward = GetForwardDirection();
	v3 right = glm::cross(forward, v3(0.0f, 1.0f, 0.0f));

	if (Input::IsPressed("up")) move_dir += forward;
	if (Input::IsPressed("down")) move_dir -= forward;
	if (Input::IsPressed("right")) move_dir += right;
	if (Input::IsPressed("left")) move_dir -= right;

	if (move_dir != v3(0)) {
	  //_velocity = glm::normalize(move_dir) * _speed;
	  _position += glm::normalize(move_dir) * _speed * delta;
	}

	if (Input::IsPressed("jump")) {
	  _position.y += _speed * delta;
	}
	if (Input::IsPressed("crounch")) {
	  _position.y -= _speed * delta;
	}

	Rotate(Input::MouseMotionY() * _mouse_sen, Input::MouseMotionX() * _mouse_sen);

  }
}

void player_t::UpdateCamera(camera_t& camera) {
  camera._angles.x = glm::radians(_angleX);
  camera._angles.y = glm::radians(_angleY);
  camera._position = _position;
}

void player_t::MoveForward(float distance) {
  v3 dir = GetForwardDirection();
  _position += dir * distance;
}

void player_t::Strafe(float distance) {
  v3 dir = GetForwardDirection();
  v3 right = glm::cross(dir, v3(0.0f, 1.0f, 0.0f));
  _position += right * distance;
}

void player_t::Rotate(float angleX, float angleY) {
  _angleY += angleY;
  _angleX += angleX;
  _angleX = glm::clamp(_angleX, -89.0f, 89.0f);
  //std::cout << _angleX << "\n";
}

void player_t::Jump() {
}

v3 player_t::GetLookDirection() {
  float x = glm::cos(glm::radians(_angleY)) * glm::cos(glm::radians(_angleX));
  float y = glm::sin(glm::radians(_angleX));
  float z = glm::sin(glm::radians(_angleY)) * glm::cos(glm::radians(_angleX));
  return glm::normalize(v3(x, y, z));
}

v3 player_t::GetForwardDirection() {
  float angle = -glm::radians(_angleY);
  float x = -glm::sin(angle);
  float z = -glm::cos(angle);
  //float x = glm::sin(glm::radians(_angleY)) * glm::cos(glm::radians(_angleX));
  return glm::normalize(v3(x, 0, z));
}

