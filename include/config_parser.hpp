#pragma once

#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>
#include <math.h>
#include <unordered_map>
#include <algorithm>
#include <iostream>
#include <regex>
#include <iterator>
#include "json.hpp"
#include "types.h"

using json = nlohmann::json;

#define CONFIG_ROOT "/EdiZon/editor/"

class ConfigParser {
public:
    ConfigParser();
    ~ConfigParser();

    static inline std::unordered_map<u64, bool> g_editableTitles;
    static bool hasConfig(u64 titleId);
    static bool loadConfigFile(u64 titleId, json &j, std::string filepath);

private:
    static inline json m_offsetFile;
};
