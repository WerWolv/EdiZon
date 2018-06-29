#pragma once

#include <string>
#include <functional>
#include <vector>

#include <switch.h>

class Gui;

class ListSelector {
public:
  ListSelector(Gui *gui, std::string title, std::string options, std::vector<std::string> &listItems);
  ~ListSelector();

  ListSelector* setInputAction(std::function<void(u32, u16)> inputActions);

  void draw();
  void onInput(u32 kdown);
  void onTouch(touchPosition &touch);

  void show();
  void hide();

  u16 selectedItem;

private:
  Gui *m_gui;
  std::string m_title, m_options;
  std::vector<std::string> &m_listItems;
  std::function<void(u32, u16)> m_inputActions;

  u32 m_optionsWidth, m_optionsHeight;
  bool m_isShown;
  s16 yOffset, yOffsetNext;
  s16 startYOffset;
};
