#pragma once

#include "widget.hpp"

#include <switch.h>

class WidgetValue : public Widget {
public:
  WidgetValue(u8 addressSize, u8 valueSize, u64 minValue, u64 maxValue);
  ~WidgetValue();

  void draw(Gui *gui, u16 x, u16 y);

  void onInput(u32 kdown);
  void onTouch(touchPosition &touch);

private:
  u64 m_minValue, m_maxValue;
};
