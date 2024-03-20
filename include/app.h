#pragma once 
#include "voxel_renderer.h"
#include "player.h"
#include "biome.h"
#include "terrain.h"
#include "sky.h"
#include <FastNoise/FastNoise.h>
#include <string>
#include <random>
#include <memory>
#include <unordered_map>
#include <deque>
#include <unordered_set>
#include <vector>
#include <memory>
#include <optional>
#include <set>

enum class WindowEvent : int {
  pos, size, lose_focus, on_focus, close
};
enum class ControlMode {
  fps, escaped, change
};
enum work_status_t {
  work_remain, work_completed
};

class app_t {
public:
  app_t(std::string name, int w = 800, int h = 600);
  ~app_t();

  void run();

  GLFWwindow* _main_window;
  float _frame_cap = 60.0f;

  app_t(const app_t&) = delete;

  //public to be called by glfw 
  void MouseMotionCallback(int x, int y);
  void KeyCallback(int key, int scancode, int action, int mods);
  void WindowCallback(WindowEvent event);
  void MouseButtonCallback(int button, int action, int mods);
private:
  void InitGraphic(std::string& window_name, int w, int h);
  void InitGenerators();
  void InitCallback();
  void InitProps();

  void UpdateEvents();
  void Process();
  void Draw();
  void UpdateChunkLoadOrder();
  void LoadChunk();
  void UnloadChunk();
  bool ProcessDeferred(timepoint_t deadline);
  void SetControlMode(ControlMode mode);

  player_t _player;
  camera_t _camera;
  Voxel::terrain_t _terrain;
  std::unordered_map<v2i, Voxel::chunk_t> _voxels;
  std::unordered_map<v2i, chunk_biome_t> _biome_data;
  std::optional<voxel_renderer_t> _voxel_renderer;
  std::optional<sky_renderer_t> _sky_renderer;
  std::unique_ptr<std::deque<v2i>> _chunk_load;
  std::unique_ptr<std::deque<v2i>> _chunk_update;
  std::unique_ptr<std::deque<v2i>> _chunk_unload;
  FastNoise::SmartNode<FastNoise::Simplex> _simplex;
  FastNoise::SmartNode<FastNoise::FractalFBm> _fractal;
  std::mt19937 _random_gen;
  fract_duration_t _delta;
  milliseconds _simulated_time;
  milliseconds _day_cycle;
  nanoseconds _block_time;
  v2d _mouse_pos;
  int _seed;
  int _chunk_distance;
  bool _blocking_event;
  bool _window_focus;
  ControlMode _control_mode;
};