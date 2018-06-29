#pragma once

#include "widget.hpp"

#include <switch.h>

class WidgetSwitch : public Widget {
public:
  WidgetSwitch(LuaSaveParser *saveParser, u64 onValue, u64 offValue);
  WidgetSwitch(LuaSaveParser *saveParser, std::string onValue, std::string offValue);

  ~WidgetSwitch();

  void draw(Gui *gui, u16 x, u16 y);

  void onInput(u32 kdown);
  void onTouch(touchPosition &touch);

private:
  u64 m_intOnValue, m_intOffValue;
  std::string m_strOnValue, m_strOffValue;

};
