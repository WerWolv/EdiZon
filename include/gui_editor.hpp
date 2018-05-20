#pragma once

#include "gui.hpp"
#include "title.hpp"

class GuiEditor : public Gui {
public:
  GuiEditor(Title *title);
  ~GuiEditor();

  void draw();
  void onInput(u32 kdown);

private:
  Title* m_currentTitle;
};
