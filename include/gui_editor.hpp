#pragma once

#include "gui.hpp"
#include "widget.hpp"
#include <vector>
#include <tuple>

class GuiEditor : public Gui {
public:
  GuiEditor();
  ~GuiEditor();

  void draw();
  void onInput(u32 kdown);
  void onTouch(touchPosition &touch);

private:
  WidgetList m_widgets;
  std::vector<std::tuple<std::string, size_t, u8*>> m_files;
};
