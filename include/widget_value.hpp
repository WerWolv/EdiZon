#pragma once

#include "widget.hpp"

#include <switch.h>

class WidgetValue : public Widget {
public:
  WidgetValue(LuaSaveParser *saveParser, s32 minValue, s32 maxValue, u32 stepSize);
  ~WidgetValue();

  void draw(Gui *gui, u16 x, u16 y);

  void onInput(u32 kdown);
  void onTouch(touchPosition &touch);

private:
  s32 m_minValue, m_maxValue;
  u32 m_stepSize;
};
