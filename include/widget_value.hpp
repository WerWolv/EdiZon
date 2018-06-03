#pragma once

#include "widget.hpp"

#include <switch.h>

class WidgetValue : public Widget {
public:
  WidgetValue(u16 minValue, u16 maxValue);
  ~WidgetValue();

  void draw(Gui *gui, u16 x, u16 y);

  void onInput(u32 kdown);
  void onTouch(touchPosition &touch);

private:
  u16 m_minValue, m_maxValue;
};
