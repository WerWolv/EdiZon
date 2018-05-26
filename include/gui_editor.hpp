#pragma once

#include "gui.hpp"
#include "title.hpp"

#include "widget.hpp"
#include "widget_switch.hpp"

#include <unordered_map>

class GuiEditor : public Gui {
public:
  GuiEditor();
  ~GuiEditor();

  void draw();
  void onInput(u32 kdown);
  void onTouch(touchPosition &touch);

private:
  WidgetList m_widgets;
};
