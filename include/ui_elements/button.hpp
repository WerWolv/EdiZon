#pragma once

#include <edizon.h>

#include <vector>

#include "guis/gui.hpp"

class Button;
typedef struct {
  std::vector<Button*> buttons;
  s8 selectedButton;
} buttonGroup_t;

class Button {
public:
  Button(u16 x, u16 y, u16 w, u16 h, std::string text, buttonGroup_t *buttonGroup);
  ~Button();

  void draw(Gui *gui);
  void onInput(u32 kdown);
  void onTouch(touchPosition &touch);

  Button* setInputAction(std::function<void(u32 kdown)> inputActions);

private:
  u16 m_x, m_y, m_w, m_h;
  std::string m_text;
  buttonGroup_t *m_buttonGroup;

  std::function<void(u32 kdown)> m_inputActions;
};


