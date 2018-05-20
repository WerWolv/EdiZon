#pragma once

#include "gui.hpp"

#include <vector>

class GuiMain : public Gui {
public:
  GuiMain();
  ~GuiMain();

  void draw();
  void onInput(u32 kdown);

private:
  u8 m_selectedItem;
  u64 m_selectedTitleId;
};
