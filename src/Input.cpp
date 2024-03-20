#include "input.h"
#include <string>
#include "debug.h"
#include <array>

namespace Input {
  struct state_singleton_t {
	std::array<bool, input_total> pressed;
	std::array<bool, input_total> just_pressed;
	std::array<bool, input_total> just_released;
	std::array<int, 2> mouse_position;
	std::array<int, 2> mouse_motion;
	std::array<std::string, input_total> names;

	input_t NameToType(std::string_view name) {
	  for (int i = 0; i < input_total; i++)
		if (name == names[i]) return (input_t)i;

	  return err;
	}
  };

  static state_singleton_t state;


  bool IsPressed(input_t type) { return state.pressed[type]; }
  bool IsJustPressed(input_t type) { return state.just_pressed[type]; }
  bool IsReleased(input_t type) { return !state.pressed[type]; }
  bool IsJustReleased(input_t type) { return state.just_released[type]; }

  bool IsPressed(std::string_view name) { return IsPressed(state.NameToType(name)); }
  bool IsJustPressed(std::string_view name) { return IsJustPressed(state.NameToType(name)); }
  bool IsReleased(std::string_view name) { return IsReleased(state.NameToType(name)); }
  bool IsJustReleased(std::string_view name) { return IsJustReleased(state.NameToType(name)); }

  int MouseMotionX() {
	return state.mouse_motion[0];
  }
  int MouseMotionY() {
	return state.mouse_motion[1];
  }

  void SetNewFrame() {
	state.just_pressed.fill(false);
	state.just_released.fill(false);

	for (int i : {0, 1}) {
	  state.mouse_position[i] += state.mouse_motion[i];
	  state.mouse_motion[i] = 0;
	}
  }

  void SetStatePressed(input_t type, std::string_view name) {
	assert(type < input_total);
	state.names[type] = name;

	if (state.pressed[type]) return;

	state.just_pressed[type] = true;
	state.pressed[type] = true;
  }

  void SetStateReleased(input_t type, std::string_view name) {
	assert(type < input_total);
	state.names[type] = name;

	if (!state.pressed[type]) return;

	state.just_released[type] = true;
	state.pressed[type] = false;
  }
  void Input::AddMouseMotion(int x, int y) {
	state.mouse_motion[0] += x;
	state.mouse_motion[1] += y;
  }
}