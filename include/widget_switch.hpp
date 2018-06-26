#pragma once

#include "widget.hpp"

#include <switch.h>

class WidgetSwitch : public Widget {
public:
  WidgetSwitch(u8 addressSize, u8 valueSize, u64 onValue, u64 offValue);
  ~WidgetSwitch();

  void draw(Gui *gui, u16 x, u16 y);

  void onInput(u32 kdown);
  void onTouch(touchPosition &touch);

private:
  u64 m_onValue, m_offValue;

};
