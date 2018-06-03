#pragma once

#include "widget.hpp"

#include <switch.h>

class WidgetSwitch : public Widget {
public:
  WidgetSwitch();
  ~WidgetSwitch();

  void draw(Gui *gui, u16 x, u16 y);

  void onInput(u32 kdown);
  void onTouch(touchPosition &touch);
};
