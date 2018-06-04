#pragma once

#include "gui.hpp"

#include <vector>
#include <unordered_map>

#define MENU_SCROLL_SPEED 40

class GuiMain : public Gui {
public:
  GuiMain();
  ~GuiMain();

  void draw();
  void onInput(u32 kdown);
  void onTouch(touchPosition &touch);

private:
  struct {
    u8 titleIndex;
    u8 accountIndex;
    u8 saveFileIndex;

    u64 titleId;
    u128 userId;
  } m_selected;

};
