#include "app.h"
#include "nlohmann/json.hpp"
#include "input.h"
#include <filesystem>
#include <fstream>

using path_t = std::filesystem::path;
using json_t = nlohmann::json;
using namespace nlohmann;

int main() {
  json_t config;
  std::fstream f(RESOURCES_PATH"config.json");
  if (f) {
	config = json_t::parse(f);
	f.close();
  }
  else {
	config["w"] = 1700;
	config["h"] = 1000;
	config["title"] = "game";
  }

	app_t app(config["title"], config["w"].get<int>(), config["h"].get<int>());
	app.run();

	std::ofstream new_config(RESOURCES_PATH "config.json");
	new_config << config.dump();
	new_config.close();

  return 0;
}
