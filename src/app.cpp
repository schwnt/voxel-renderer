#include "app.h"
#include "input.h"
#include "debug.h"
#include "api.h"
#include <thread>
#include <random>
#include <ranges>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <format>
#include <algorithm>
#include <glm/gtx/string_cast.hpp>

app_t::app_t(std::string name, int w, int h) {

  InitGraphic(name, w, h);
  InitGenerators();
  InitProps();
  InitCallback();

}

void app_t::InitGraphic(std::string& window_name, int w, int h) {
  if (!glfwInit())
	assert(false);

  glfwWindowHint(GLFW_FOCUSED, GLFW_FALSE);

  _main_window = glfwCreateWindow(w, h, window_name.c_str(), nullptr, nullptr);
  glfwSetWindowPos(_main_window, 0, 50);

  assert(_main_window);

  glfwMakeContextCurrent(_main_window);

  gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

  if constexpr (DBG_GL_V) {
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(glDebug::glDebugOutput, 0);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
  }

  Voxel::TextureLoader texture_loader;
  _voxel_renderer.emplace(_main_window, texture_loader);

  /* sky renderer */
  std::ifstream f(RESOURCES_PATH "resource.json");
  assert(f);
  json_t res_config = json_t::parse(f);
  f.close();

  json_t& sky_res = res_config["sky"][res_config["use"]["sky"]];

  path_t sky_dir = path_t(RESOURCES_PATH) / sky_res["path"];
  sky_dir.make_preferred();

  std::vector<std::string> sky_names;
  std::vector<path_t> sky_paths;

  for (auto& [name, img] : sky_res["images"].items()) {
	path_t sky_path = path_t(sky_dir / img).make_preferred();
	assert(fs::exists(sky_path));
	sky_paths.emplace_back(sky_path);
	sky_names.emplace_back(name);
  }

  _sky_renderer.emplace(sky_paths, sky_names);
}

void app_t::InitGenerators() {
  _seed = std::random_device{}();
  _random_gen.seed(_seed);

  _simplex = FastNoise::New<FastNoise::Simplex>();
  _fractal = FastNoise::New<FastNoise::FractalFBm>();
  _fractal->SetSource(_simplex);
}

void app_t::InitCallback() {
  glfwSetWindowUserPointer(_main_window, static_cast<void*>(this));

  glfwSetKeyCallback(_main_window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
	app_t* app = static_cast<app_t*>(glfwGetWindowUserPointer(window));
	app->KeyCallback(key, scancode, action, mods);
	});

  glfwSetCursorPosCallback(_main_window, [](GLFWwindow* window, double xpos, double ypos) {
	app_t* app = static_cast<app_t*>(glfwGetWindowUserPointer(window));
	app->MouseMotionCallback(xpos, ypos);
	});

  glfwSetMouseButtonCallback(_main_window, [](GLFWwindow* window, int button, int action, int mods) {
	app_t* app = static_cast<app_t*>(glfwGetWindowUserPointer(window));
	app->MouseButtonCallback(button, action, mods);
	});

  glfwSetWindowPosCallback(_main_window, [](GLFWwindow* window, int xpos, int ypos) {
	app_t* app = static_cast<app_t*>(glfwGetWindowUserPointer(window));
	app->WindowCallback(WindowEvent::pos);
	});

  glfwSetWindowSizeCallback(_main_window, [](GLFWwindow* window, int width, int height) {
	app_t* app = static_cast<app_t*>(glfwGetWindowUserPointer(window));
	app->WindowCallback(WindowEvent::size);
	});

  glfwSetWindowCloseCallback(_main_window, [](GLFWwindow* window) {
	app_t* app = static_cast<app_t*>(glfwGetWindowUserPointer(window));
	app->WindowCallback(WindowEvent::close);
	});

  glfwSetWindowFocusCallback(_main_window, [](GLFWwindow* window, int focused) {
	app_t* app = static_cast<app_t*>(glfwGetWindowUserPointer(window));
	if (focused) app->WindowCallback(WindowEvent::on_focus);
	else app->WindowCallback(WindowEvent::lose_focus);
	});
}

void app_t::InitProps() {
  _chunk_load = std::make_unique<decltype(_chunk_load)::element_type>();
  _chunk_unload = std::make_unique<decltype(_chunk_unload)::element_type>();
  _chunk_update = std::make_unique<decltype(_chunk_update)::element_type>();

  _player = player_t();
  _player._position = v3(0.0f, 200.0f, 0.0f);
  _player._mouse_sen = 0.1f;
  _chunk_distance = 9;
  _blocking_event = false;
  _delta = fract_duration_t::zero();
  _block_time = 0ns;
  glfwGetCursorPos(_main_window, &_mouse_pos.x, &_mouse_pos.y);
  SetControlMode(ControlMode::escaped);

  _terrain._seed = std::uniform_int_distribution(0, 90000)(_random_gen);
  _terrain._layers = {
		{Block::stone, 20,50},
		{Block::dirt, 10, 3},
		{Block::grass, 3,0},
  };

  _camera._fovy = 60.0f;
  _camera = camera_t(v3(), 90.0f, 1.0f);
  _camera._far = 20000.0f;

  json_t setting;
  std::ifstream f(RESOURCES_PATH "setting.json");
  assert(f);
  if (f) {
	setting = json_t::parse(f);
	f.close();
  }
  else {
	setting["day_cycle"] = 300;
	setting["chunk_distance"] = 8;
	setting["player_speed"] = 25;
  }

  if (setting.contains("player_speed"))
	_player._speed = setting["player_speed"].get<float>();
  else
	_player._speed = 25.0f;

  if (setting.contains("day_cycle"))
	_day_cycle = seconds(setting["day_cycle"].get<int>());
  else
	_day_cycle = 5min;

  if (setting.contains("chunk_distance"))
	_chunk_distance = setting["chunk_distance"].get<int>();
  else 
	_chunk_distance = 8;

  std::ofstream new_setting(RESOURCES_PATH "setting.json");
  new_setting << setting.dump();
  new_setting.close();
}

void app_t::WindowCallback(WindowEvent event) {
  switch (event) {
  case WindowEvent::pos:
	_blocking_event = true;
	break;
  case WindowEvent::size:
	_blocking_event = true;
	int w, h;
	glfwGetWindowSize(_main_window, &w, &h);
	glViewport(0, 0, w, h);
	_camera._aspect = float(w) / h;
	break;
  case WindowEvent::lose_focus:
	_window_focus = false;
	_blocking_event = true;
	break;
  case WindowEvent::on_focus:
	_window_focus = true;
	break;
  case WindowEvent::close:
	glfwWindowShouldClose(_main_window);
	break;
  default:
	break;
  }
}

void app_t::MouseMotionCallback(int x, int y) {
  if (!_window_focus || _control_mode == ControlMode::escaped) return;

  int mx = x - (int)_mouse_pos.x;
  int my = y - (int)_mouse_pos.y;
  Input::AddMouseMotion(mx, my);
  _mouse_pos = { x,y };
}

void app_t::MouseButtonCallback(int button, int action, int mods) {
  return;
  if (!_window_focus || _control_mode == ControlMode::escaped) return;

  std::string_view name = "any";
  Input::input_t type = Input::button0;
  if (button == GLFW_MOUSE_BUTTON_LEFT) {
	type = Input::button0;
	name = "button0";
  }
  else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
	type = Input::button1;
	name = "button1";
  }

  if (action == GLFW_PRESS) {
	Input::SetStatePressed(Input::any, "any");
	Input::SetStatePressed(type, name);
  }
  else if (action == GLFW_RELEASE) {
	Input::SetStateReleased(Input::any, "any");
	Input::SetStateReleased(type, name);
  }
}

void app_t::KeyCallback(int key, int scancode, int action, int mods) {
  if (!_window_focus) return;

  if (key == GLFW_KEY_ESCAPE) {
	if (action == GLFW_PRESS) SetControlMode(ControlMode::change);
	return;
  }

  if (_control_mode == ControlMode::escaped) return;

  Input::input_t type = Input::any;
  std::string_view name = "any";

  switch (key) {
  case GLFW_KEY_W:
	type = Input::up;
	name = "up";
	break;
  case GLFW_KEY_A:
	type = Input::left;
	name = "left";
	break;
  case GLFW_KEY_S:
	type = Input::down;
	name = "down";
	break;
  case GLFW_KEY_D:
	type = Input::right;
	name = "right";
	break;
  case GLFW_KEY_SPACE:
	type = Input::jump;
	name = "jump";
	break;
  case GLFW_KEY_LEFT_CONTROL:
	type = Input::crounch;
	name = "crounch";
	break;
  case GLFW_KEY_0:
	type = Input::n0;
	name = "num 0";
	break;
  case GLFW_KEY_1:
	type = Input::n1;
	name = "num 1";
	break;
  case GLFW_KEY_2:
	type = Input::n2;
	name = "num 2";
	break;
  case GLFW_KEY_3:
	type = Input::n3;
	name = "num 3";
	break;
  case GLFW_KEY_4:
	type = Input::n4;
	name = "num 4";
	break;
  case GLFW_KEY_5:
	type = Input::n5;
	name = "num 5";
	break;
  case GLFW_KEY_6:
	type = Input::n6;
	name = "num 6";
	break;
  case GLFW_KEY_7:
	type = Input::n7;
	name = "num 7";
	break;
  case GLFW_KEY_8:
	type = Input::n8;
	name = "num 8";
	break;
  case GLFW_KEY_9:
	type = Input::n9;
	name = "num 9";
	break;
  case GLFW_KEY_MINUS:
	type = Input::minus;
	name = "minus";
	break;
  case GLFW_KEY_EQUAL:
	type = Input::plus;
	name = "plus";
	break;
  case GLFW_KEY_PERIOD:
	type = Input::dot;
	name = "dot";
	break;
  case GLFW_KEY_COMMA:
	type = Input::comma;
	name = "comma";
	break;
  case GLFW_KEY_L:
	type = Input::l;
	name = "L";
	break;
  case GLFW_KEY_K:
	type = Input::k;
	name = "K";
	break;
  default:
	break;
  }

  if (action == GLFW_PRESS) {
	Input::SetStatePressed(Input::any, "any");
	Input::SetStatePressed(type, name);
  }
  else if (action == GLFW_RELEASE) {
	Input::SetStateReleased(Input::any, "any");
	Input::SetStateReleased(type, name);
  }
}

void app_t::run() {

  UpdateChunkLoadOrder();
  timepoint_t world_load_start = chrono_clock::now();
  while (ProcessDeferred(chrono_clock::now() + 6s)) {
	//std::cout << std::format("too long...\nchunk_load: {}  ||  chunk_update: {}  ||  chunk_unload : {}\n",
//	  _chunk_load->size(), _chunk_update->size(), _chunk_unload->size());
  }
  // std::cout << std::format("world loaded after {} [{} chunks at distance={}]\n", std::chrono::duration_cast<seconds>(chrono_clock::now() - world_load_start), _voxels.size(), _chunk_distance);

  WindowCallback(WindowEvent::size);
  glfwFocusWindow(_main_window);
  SetControlMode(ControlMode::fps);

  timepoint_t last_frame = chrono_clock::now();
  timepoint_t current_frame = chrono_clock::now();
  _simulated_time = 0ms;

  while (!glfwWindowShouldClose(_main_window)) {
	if (_blocking_event) {
	  last_frame += _block_time;
	  current_frame += _block_time;
	}
	clock_duration_t capped_frame_time = std::chrono::duration_cast<clock_duration_t>(nanoseconds(1s) * (1.0f / _frame_cap));
	timepoint_t frame_should_start = last_frame + capped_frame_time;

	using namespace std::literals::string_view_literals;
	if (!ProcessDeferred(frame_should_start))
	  std::this_thread::sleep_until(frame_should_start);
	//  std::cout << "true\n"sv;

	last_frame = std::exchange(current_frame, chrono_clock::now());

	_delta = current_frame - last_frame;
	if (_delta < 5ms) _delta = 5ms;

	if (Input::IsPressed("comma")) {
	  _simulated_time += std::chrono::duration_cast<milliseconds>(_day_cycle / (10 * (1s / _delta)));
	}

	_simulated_time += std::chrono::duration_cast<milliseconds>(_delta);
	if (_simulated_time > _day_cycle) {
	  _simulated_time %= _day_cycle;
	}

	v3 old_playerpos = _player._position;

	Process();
	Draw();

	int old_distance = _chunk_distance;
	if (Input::IsJustPressed("plus")) {
	  //	  std::cout << std::format("distance={}\n", _chunk_distance);
	  _chunk_distance++;
	}
	if (Input::IsJustPressed("minus")) {
	  //  std::cout << std::format("distance={}\n", _chunk_distance);
	  _chunk_distance--;
	}
	//if (Input::IsJustPressed("dot")) {
	//  int v[3];
	//  _terrain.GetElevationDBG(_player._position.x, _player._position.z, v[0], v[1], v[2]);
	//  v2i local_pos = v2i(_player._position.x, _player._position.y) - Voxel::GetChunkPos(_player._position);
	//  float temp = _biome_data[v2i(Voxel::GetChunkPos(_player._position))].temperature[local_pos.x + local_pos.y*16];
	//  std::cout << std::format("pos={} continential={}, erosion={}, pv={}, temp={} look={}\n", glm::to_string(_player._position),
	//	v[0], v[1], v[2], temp, glm::to_string(_camera.GetLook()));
	//}

	if (Voxel::GetChunkPos(_player._position) != Voxel::GetChunkPos(old_playerpos) || _chunk_distance != old_distance) {
	  UpdateChunkLoadOrder();
	}

	glfwSwapBuffers(_main_window);
	UpdateEvents();
  }
}

void app_t::UpdateEvents() {
  Input::SetNewFrame();

  _blocking_event = false;
  timepoint_t poll_begin = chrono_clock::now();

  glfwPollEvents();
  while (!_window_focus) {
	glfwPollEvents();
	std::this_thread::sleep_for(10ms);
  }

  if (_blocking_event) {
	_block_time = chrono_clock::now() - poll_begin;
  }
}

void app_t::Process() {
  if (Input::IsJustPressed("num 5")) {
	_camera._fovy += 10.0f;
  }
  if (Input::IsJustPressed("num 6")) {
	_camera._fovy -= 10.0f;
  }
  _player.Process(_delta.count(), true);

}

void app_t::Draw() {
  _player.UpdateCamera(_camera);

  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  float skylight_angle = 2 * glm::pi<double>() * ((double)_simulated_time.count() / _day_cycle.count());
  v3 skylight_rotz = glm::rotate(skylight_angle, v3(0, 0, 1.0f)) * v4(-1.0f, 0, 0, 0);
  v3 skylight_roty = glm::rotate(skylight_angle, v3(0, 1.0f, 0.0)) * v4(-1.0f, 0, 0, 0);
  v3 skylight = glm::normalize(skylight_rotz + skylight_roty);
  _voxel_renderer->Draw_All(_camera, skylight);

  float day_factor = 0.5f + glm::dot(skylight, v3(0, -1.0f, 0));
  day_factor = glm::clamp(day_factor, 0.0f, 1.0f);
  float dusk_factor = 1.0f - glm::abs(glm::dot(skylight, v3(0, -1.0f, 0)));
  dusk_factor = day_factor * glm::pow(dusk_factor, 3.0f) * 0.8f;
  day_factor -= dusk_factor;
  float night_factor = 1.0f - day_factor - dusk_factor;

  _sky_renderer->DrawMix(_camera, { "day", "orange", "night" }, { day_factor, dusk_factor,night_factor });
}

bool app_t::ProcessDeferred(timepoint_t deadline) {
  while (!_chunk_load->empty()) {
	LoadChunk();
	if (chrono_clock::now() > deadline)
	  return true;
  }
  while (!_chunk_update->empty()) {
	v2i p = _chunk_update->back();
	_voxel_renderer->Update_Chunk(p[0], p[1], _voxels[p], _biome_data[p]);
	_chunk_update->pop_back();
	if (chrono_clock::now() > deadline)
	  return true;
  }
  while (!_chunk_unload->empty()) {
	UnloadChunk();
	if (chrono_clock::now() > deadline)
	  return true;
  }
  return false;
}

void app_t::SetControlMode(ControlMode mode) {
  switch (mode)
  {
  case ControlMode::fps:
	glfwSetInputMode(_main_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPos(_main_window, _mouse_pos.x, _mouse_pos.y);
	_control_mode = mode;
	break;
  case ControlMode::escaped:
	glfwSetInputMode(_main_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	_control_mode = mode;
	break;
  case ControlMode::change:
	if (_control_mode == ControlMode::fps)
	  SetControlMode(ControlMode::escaped);
	else
	  SetControlMode(ControlMode::fps);
	break;
  default:
	break;
  }
}

void app_t::LoadChunk() {
  if (_chunk_load->empty()) return;

  v2i p = _chunk_load->front();
  int x = p[0], z = p[1];

  assert(!_voxels.contains(p));

  chunk_biome_t& biome = _biome_data[p];
  _simplex->GenUniformGrid2D(biome.temperature.data(), p.x, p.y, 16, 16, 0.0001f, _seed);

  Voxel::chunk_t& chunk = _voxels[p];
  _terrain.GenerateChunk(chunk, x, z, biome);

  _chunk_load->pop_front();

  if (std::ranges::find(*_chunk_update, p) == _chunk_update->end())
	_chunk_update->push_front(p);

  for (v2i adj_dir : {v2i(1, 0), v2i(-1, 0), v2i(0, 1), v2i(0, -1)}) {
	v2i neighbor_pos = p + adj_dir * 16;
	if (_voxels.contains(neighbor_pos)) {
	  chunk.SetNeighbor(adj_dir, _voxels[neighbor_pos]);

	  if (std::ranges::find(*_chunk_update, neighbor_pos) == _chunk_update->end())
		_chunk_update->push_front(neighbor_pos);
	}
  }

}

void app_t::UnloadChunk() {
  if (_chunk_unload->empty()) return;

  v2i p = _chunk_unload->front();
  int x = p[0], z = p[1];

  _voxels[p].UnsetNeighbor();

  _voxels.erase(p);
  _voxel_renderer->Free_Chunk(x, z);

  _biome_data.erase(p);

  _chunk_unload->pop_front();
}

void app_t::UpdateChunkLoadOrder() {
  v2i player_p = Voxel::GetChunkPos(_player._position);
  int d = _chunk_distance;

  auto new_load = std::make_unique<decltype(_chunk_load)::element_type>();
  auto new_unload = std::make_unique<decltype(_chunk_unload)::element_type>();

  for (int x = player_p[0] - d * 16; x <= player_p[0] + d * 16; x += 16)
	for (int z = player_p[1] - d * 16; z <= player_p[1] + d * 16; z += 16) {
	  v2i p(x, z);
	  if (_voxels.contains(p)) continue;

	  if (std::ranges::find(*new_load, p) == new_load->end())
		new_load->push_back(p);
	}


  std::ranges::sort(*new_load, [player_p](v2i a, v2i b) {
	return glm::length(v2(a - player_p)) < glm::length(v2(b - player_p));
	});

  // std::cout << glm::to_string(_player._position) << "\n";
  for (v2i k : _voxels | std::views::keys) {
	v2i p = k - player_p;

	if (glm::max(abs(p[0]), abs(p[1])) > (_chunk_distance + 2) * 16) {
	  new_unload->push_back(k);
	  // std::cout << glm::to_string(k) << "\n";
	}
  }

  std::ranges::sort(*new_unload, [player_p](v2i a, v2i b) {
	return glm::length(v2(a - player_p)) > glm::length(v2(b - player_p));
	});

  _chunk_load.swap(new_load);
  _chunk_unload.swap(new_unload);
}

app_t::~app_t() {
  glfwDestroyWindow(_main_window);
  glfwTerminate();
}
