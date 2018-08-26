#pragma once

#include "gui.hpp"
#include "widget.hpp"

#include "json.hpp"

using json = nlohmann::json;

#define CONFIG_ROOT "/EdiZon/editor/"

class GuiEditor : public Gui {
public:
  GuiEditor();
  ~GuiEditor();

  void update();
  void draw();
  void onInput(u32 kdown);
  void onTouch(touchPosition &touch);
  void onGesture(touchPosition &startPosition, touchPosition &endPosition);

  static inline u8 *g_currSaveFile = nullptr;
  static inline std::string g_currSaveFileName = "";

private:
  WidgetItems m_widgets;
  json m_offsetFile;

  u8* m_titleIcon;

  std::vector<std::string> m_backupNames;
  std::vector<std::string> m_saveFiles;

  color_t m_dominantColor;
  color_t m_textColor;

  s8 m_configFileResult;

  LuaSaveParser m_luaParser;

  void createWidgets();
  void updateBackupList();

  s8 loadConfigFile(json &j, std::string filepath);
  void updateSaveFileList(std::vector<std::string> saveFilePath, std::string files);
};
