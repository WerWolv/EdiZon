#include "config_parser.hpp"
#include "title.hpp"

ConfigParser::ConfigParser() {
  
}

bool ConfigParser::hasConfig(u64 titleId) {
    std::stringstream path;
    path << CONFIG_ROOT << std::setfill('0') << std::setw(sizeof(u64) * 2) << std::uppercase << std::hex << titleId << ".json";

    return ConfigParser::loadConfigFile(titleId, m_offsetFile, path.str());
}

bool ConfigParser::loadConfigFile(u64 titleId, json &j, std::string filepath) {
  std::ifstream file(filepath.c_str());

  if (file.fail())
    return false;

  try {
    file >> j;
  } catch (json::parse_error& e) {
		printf("Failed to parse JSON file.\n");
		return false;
	}

  if (j.find("useInstead") != j.end()) {
    std::stringstream path;
    path << CONFIG_ROOT << j["useInstead"].get<std::string>();
    return ConfigParser::loadConfigFile(titleId, j, path.str());
  }

  bool foundVersion = false;

  if (j.find("all") == j.end()) {
    for (auto it : j.items()) {
      if (it.key().find(Title::g_titles[titleId]->getTitleVersion()) != std::string::npos) {
        foundVersion = true;
      }
    }
  } else {
    foundVersion = true;
  }

  if (!foundVersion)
    return false;

  return true;
}
