#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "widget.hpp"
#include "types.h"

#define CONFIG_ROOT "/EdiZon/editor/"

class ScriptParser;

class ConfigParser {
public:
    ConfigParser() = delete;

    static s8 hasConfig(u64 titleId);
    static s8 loadConfigFile(u64 titleId, std::string filepath);
    static void unloadConfigFile();
    static void createWidgets(WidgetItems &widgets, ScriptParser &scriptParser);

    static std::string getString(std::vector<std::string> keys);
    static std::vector<std::string> getStrings(std::vector<std::string> keys);
    static std::string getOptionalString(std::vector<std::string> keys, std::string optionalKey, std::string elseVal);
    static u64 getOptionalInt(std::vector<std::string> keys, std::string optionalKey, u64 elseVal);

    static inline std::unordered_map<u64, bool> g_editableTitles;

};
