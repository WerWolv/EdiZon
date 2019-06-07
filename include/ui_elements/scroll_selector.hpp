#pragma once

#include <edizon.h>

#include "guis/gui.hpp"

#include <vector>
#include <string>

class ScrollSelector {
public:
  ScrollSelector(u16 x, u16 y, std::vector<std::string> options);
  ~ScrollSelector();

  void draw(Gui *gui);
  void onInput(u32 kdown);
  void onTouch(touchPosition &touch);

private:
  u16 m_x, m_y;
  std::vector<std::string> m_options;
  u16 m_currSelection;

};