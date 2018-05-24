#pragma once

#include "gui.hpp"

#include <vector>
#include <unordered_map>

class GuiMain : public Gui {
public:
  GuiMain();
  ~GuiMain();

  void draw();
  void onInput(u32 kdown);

private:
  struct {
    u8 titleIndex;
    u8 accountIndex;
    u8 saveFileIndex;

    u64 titleId;
    u128 userId;
  } m_selected;

};
