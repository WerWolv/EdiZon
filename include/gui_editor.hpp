#pragma once

#include "gui.hpp"
#include "widget.hpp"

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

  void updateSaveFileList(std::vector<std::string> saveFilePath, std::string files);
};
