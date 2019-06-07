#pragma once

#include "widgets/widget.hpp"

#include <edizon.h>

class WidgetSwitch : public Widget {
public:
  WidgetSwitch(Interpreter *interpreter, bool isDummy, std::string tooltip, s32 onValue, s32 offValue);
  WidgetSwitch(Interpreter *interpreter, bool isDummy, std::string tooltip, std::string onValue, std::string offValue);

  ~WidgetSwitch();

  void draw(Gui *gui, u16 x, u16 y);

  void onInput(u32 kdown);
  void onTouch(touchPosition &touch);

private:
  s32 m_intOnValue, m_intOffValue;
  std::string m_strOnValue, m_strOffValue;

};
