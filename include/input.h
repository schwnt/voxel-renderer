#pragma once
#include <string_view>

namespace Input {
  enum input_t {
	any, button0, button1, left, right, up, down, jump, crounch,
	n1, n2, n3, n4, n5, n6, n7, n8, n9, n0,
	minus, plus, equal, squareleft, squareright, enter, colon, question, comma, dot,
	l, k, q, w, e, r, t, y, u, i, o, p, a, s, d, f, g, h, j, z, x, c, v, b, n, m,
	input_total, err
  };

  bool IsPressed(input_t type);
  bool IsJustPressed(input_t type);
  bool IsReleased(input_t type);
  bool IsJustReleased(input_t type);

  bool IsPressed(std::string_view name);
  bool IsJustPressed(std::string_view name);
  bool IsReleased(std::string_view name);
  bool IsJustReleased(std::string_view name);

  int MouseMotionX();
  int MouseMotionY();

  void SetNewFrame();
  void SetStatePressed(input_t type, std::string_view name);
  void SetStateReleased(input_t type, std::string_view name);
  void AddMouseMotion(int x, int y);
}