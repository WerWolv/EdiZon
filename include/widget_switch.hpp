#pragma once

#include "widget.hpp"

class WidgetSwitch : public Widget {
public:
  WidgetSwitch();
  ~WidgetSwitch();

  void draw(Gui *gui, u16 x, u16 y);

  void onInput(u32 kdown);
  void onTouch(touchPosition &touch, u16 widgetX, u16 widgetY);
};
