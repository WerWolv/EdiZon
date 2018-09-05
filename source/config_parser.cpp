#include "config_parser.hpp"
#include "title.hpp"

ConfigParser::ConfigParser() {
  
}

s8 ConfigParser::hasConfig(u64 titleId) {
    std::stringstream path;
    path << CONFIG_ROOT << std::setfill('0') << std::setw(sizeof(u64) * 2) << std::uppercase << std::hex << titleId << ".json";

    return ConfigParser::loadConfigFile(titleId, m_offsetFile, path.str());
}

s8 ConfigParser::loadConfigFile(u64 titleId, json &j, std::string filepath) {
  std::ifstream file(filepath.c_str());
  if (file.fail()) {
    printf("Failed reading the config file.\n");
    return 1;
  }

  try {
    file >> j;
  } catch (json::parse_error& e) {
		printf("Failed to parse JSON file.\n");
		return 2;
	}

  if (j.find("useInstead") != j.end()) {
    std::stringstream path;
    path << CONFIG_ROOT << j["useInstead"].get<std::string>();
    return ConfigParser::loadConfigFile(titleId, j, path.str());
  }

  if (j.find("all") == j.end()) {
    for (auto it : j.items()) {
      printf("key: %s, title version: %s\n", it.key().c_str(), Title::g_titles[titleId]->getTitleVersion().c_str());
      if (it.key().find(Title::g_titles[titleId]->getTitleVersion()) != std::string::npos) {
        j = j[it.key()];
        return 0;
      }
    }
    return 3;
  } else {
    j = j["all"];
    return 0;
  }
}
