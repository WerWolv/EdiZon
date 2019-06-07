#pragma once

#include "widgets/widget.hpp"

#include <edizon.h>

class WidgetButton : public Widget {
public:
  WidgetButton(Interpreter *interpreter, std::string tooltip, std::string funcName);

  ~WidgetButton();

  void draw(Gui *gui, u16 x, u16 y);

  void onInput(u32 kdown);
  void onTouch(touchPosition &touch);

private:
  std::string m_funcName;
  std::string m_displayText;

};
