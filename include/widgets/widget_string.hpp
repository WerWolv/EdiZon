#pragma once

#include "widgets/widget.hpp"

#include <edizon.h>

class WidgetString : public Widget {
public:
  WidgetString(Interpreter *interpreter, bool isDummy, std::string tooltip, u8 minLength, u8 maxLength);
  ~WidgetString();

  void draw(Gui *gui, u16 x, u16 y);

  void onInput(u32 kdown);
  void onTouch(touchPosition &touch);

private:
  u8 m_minLength, m_maxLength;
  
};
