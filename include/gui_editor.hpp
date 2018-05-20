#pragma once

#include "gui.hpp"
#include "title.hpp"

class GuiEditor : public Gui {
public:
  GuiEditor();
  ~GuiEditor();

  void draw();
  void onInput(u32 kdown);
};
