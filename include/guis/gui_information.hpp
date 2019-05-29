#pragma once

#include "guis/gui.hpp"

#include <string>

class GuiInformation : public Gui {
public:
  GuiInformation();
  ~GuiInformation();

  void update();
  void draw();
  void onInput(u32 kdown);
  void onTouch(touchPosition &touch);
  void onGesture(touchPosition startPosition, touchPosition endPosition, bool finish);
};
