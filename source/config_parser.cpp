#include "config_parser.hpp"
#include "title.hpp"

#include "widget_switch.hpp"
#include "widget_value.hpp"
#include "widget_list.hpp"

#include <sstream>
#include <fstream>
#include <math.h>
#include <algorithm>
#include <iostream>
#include <regex>
#include <iterator>
#include "json.hpp"

using json = nlohmann::json;

static json configFile;

std::string ConfigParser::getOptionalString(std::vector<std::string> keys, std::string optionalKey, std::string elseVal) {
  return elseVal;
}

u64 ConfigParser::getOptionalInt(std::vector<std::string> keys, std::string optionalKey, u64 elseVal) {
  return 0;
}

std::string ConfigParser::getString(std::vector<std::string> keys) {
  json j = configFile;

  for (auto key : keys) {
    printf("%s, ", key.c_str());
    j = j[key];
  }

  printf("-> %s\n", j.get<std::string>().c_str());

  return j.get<std::string>();
}

std::vector<std::string> ConfigParser::getStrings(std::vector<std::string> keys) {
  json j = configFile;

  for (auto key : keys)
    j = j[key];

  return j.get<std::vector<std::string>>();
}

s8 ConfigParser::hasConfig(u64 titleId) {
    std::stringstream path;
    path << CONFIG_ROOT << std::setfill('0') << std::setw(sizeof(u64) * 2) << std::uppercase << std::hex << titleId << ".json";

    return ConfigParser::loadConfigFile(titleId, path.str());
}

s8 ConfigParser::loadConfigFile(u64 titleId, std::string filepath) {
  std::ifstream file(filepath.c_str());
  if (file.fail())
    return 1;

  try {
    file >> configFile;
  } catch (json::parse_error& e) {
		printf("Failed to parse JSON file.\n");
		return 2;
	}

  if (configFile.find("useInstead") != configFile.end()) {
    std::stringstream path;
    path << CONFIG_ROOT << configFile["useInstead"].get<std::string>();
    return ConfigParser::loadConfigFile(titleId, path.str());
  }

  if (configFile.find("all") == configFile.end()) {
    for (auto it : configFile.items()) {
      printf("key: %s, title version: %s\n", it.key().c_str(), Title::g_titles[titleId]->getTitleVersion().c_str());
      if (it.key().find(Title::g_titles[titleId]->getTitleVersion()) != std::string::npos) {
        configFile = configFile[it.key()];

        if (configFile == nullptr) return 2;
        if (configFile["saveFilePaths"] == nullptr
            || configFile["files"] == nullptr
            || configFile["filetype"] == nullptr
            || configFile["items"] == nullptr) return 2;

        return 0;
      }
    }
    return 3;
  } else {
    configFile = configFile["all"];
    return 0;
  }
}

void ConfigParser::createWidgets(WidgetItems &widgets, ScriptParser &scriptParser) {
  std::set<std::string> tempCategories;

  if(configFile == nullptr) return;

  for (auto item : configFile["items"]) {
    if (item["name"] == nullptr || item["category"] == nullptr || item["intArgs"] == nullptr || item["strArgs"] == nullptr) continue;
    auto itemWidget = item["widget"];
    if (itemWidget == nullptr) continue;
    if (itemWidget["type"] == nullptr) continue;
    if (itemWidget["type"] == "int") {
      if (itemWidget["minValue"] == nullptr || itemWidget["maxValue"] == nullptr) continue;
      if (itemWidget["minValue"] >= itemWidget["maxValue"]) continue;
      widgets[item["category"]].push_back({ item["name"],
        new WidgetValue(&scriptParser,
          "value",
          "value",
          itemWidget["minValue"], itemWidget["maxValue"],
          0) });
    }
    else if (itemWidget["type"] == "bool") {
      if (itemWidget["onValue"] == nullptr || itemWidget["offValue"] == nullptr) continue;
      if (itemWidget["onValue"] == itemWidget["offValue"]) continue;
      if(itemWidget["onValue"].is_number() && itemWidget["offValue"].is_number()) {
        widgets[item["category"]].push_back({ item["name"],
        new WidgetSwitch(&scriptParser, itemWidget["onValue"].get<s32>(), itemWidget["offValue"].get<s32>()) });
      }
      else if(itemWidget["onValue"].is_string() && itemWidget["offValue"].is_string())
        widgets[item["category"]].push_back({ item["name"],
        new WidgetSwitch(&scriptParser, itemWidget["onValue"].get<std::string>(), itemWidget["offValue"].get<std::string>()) });
    }
    else if (itemWidget["type"] == "list") {
      if (itemWidget["listItemNames"] == nullptr || itemWidget["listItemValues"] == nullptr) continue;

      if (itemWidget["listItemValues"][0].is_number()) {
        widgets[item["category"]].push_back({ item["name"],
        new WidgetList(&scriptParser, itemWidget["listItemNames"], itemWidget["listItemValues"].get<std::vector<s32>>()) });
      }
      else if (itemWidget["listItemValues"][0].is_string())
        widgets[item["category"]].push_back({ item["name"], new WidgetList(&scriptParser, itemWidget["listItemNames"], itemWidget["listItemValues"].get<std::vector<std::string>>()) });
    }

    widgets[item["category"]].back().widget->setLuaArgs(item["intArgs"], item["strArgs"]);

    tempCategories.insert(item["category"].get<std::string>());;
    Widget::g_selectedRow = CATEGORIES;
  }

  Widget::g_categories.clear();
  std::copy(tempCategories.begin(), tempCategories.end(), std::back_inserter(Widget::g_categories));
  Widget::g_selectedCategory = Widget::g_categories[0];

  for (auto category : tempCategories)
    Widget::g_widgetPageCnt[category] = ceil(widgets[category].size() / WIDGETS_PER_PAGE);
}
