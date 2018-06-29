#pragma once

#include "widget.hpp"

#include <switch.h>

class WidgetList : public Widget {
public:
  WidgetList(Gui *gui, LuaSaveParser *saveParser, std::vector<std::string> listItemNames, std::vector<u64> listItemValues);
  WidgetList(Gui *gui, LuaSaveParser *saveParser, std::vector<std::string> listItemNames, std::vector<std::string> listItemValues);
  ~WidgetList();

  void draw(Gui *gui, u16 x, u16 y);

  void onInput(u32 kdown);
  void onTouch(touchPosition &touch);

private:
  Gui *m_gui;

  std::vector<std::string> m_listItemNames;
  std::vector<u64> m_intListItemValues;
  std::vector<std::string> m_strListItemValues;

};
