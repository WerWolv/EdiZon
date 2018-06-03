#pragma once

#include "gui.hpp"
#include "widget.hpp"

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
