#pragma once

#include "widgets/widget.hpp"

#include <edizon.h>

class WidgetList : public Widget {
public:
  WidgetList(Interpreter *interpreter, bool isDummy, std::string tooltip, std::vector<std::string> listItemNames, std::vector<s32> listItemValues);
  WidgetList(Interpreter *interpreter, bool isDummy, std::string tooltip, std::vector<std::string> listItemNames, std::vector<std::string> listItemValues);
  ~WidgetList();

  void draw(Gui *gui, u16 x, u16 y);

  void onInput(u32 kdown);
  void onTouch(touchPosition &touch);

private:
  std::vector<std::string> m_listItemNames;
  std::vector<s32> m_intListItemValues;
  std::vector<std::string> m_strListItemValues;

};
