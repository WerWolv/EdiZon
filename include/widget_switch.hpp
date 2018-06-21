#pragma once

#include "widget.hpp"

#include <switch.h>

class WidgetSwitch : public Widget {
public:
  WidgetSwitch(u16 onValue, u16 offValue, u16 offsetAddr, u16 address, u8* buffer);
  ~WidgetSwitch();

  void draw(Gui *gui, u16 x, u16 y);

  void onInput(u32 kdown);
  void onTouch(touchPosition &touch);

private:
  u16 m_onValue, m_offValue;
};
