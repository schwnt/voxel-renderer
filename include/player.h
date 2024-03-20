#pragma once
#include "commons.h"
#include "camera.h"


class player_t {
public:
  player_t() = default;

  void Process(float delta, bool input_read = true);
  void UpdateCamera(camera_t& camera);
  void MoveForward(float distance);
  void Strafe(float distance);
  void Rotate(float angleX, float angleY);
  void Jump();
  v3 GetLookDirection();
  v3 GetForwardDirection();

  v3 _position;
  v3 _velocity;
  float _angleY, _angleX;
  bool _can_jump;
  float _speed;
  float _mouse_sen;
  float _jump_force;
  float _gravity;

};

