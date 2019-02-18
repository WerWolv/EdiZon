#include "ui_elements/button.hpp"

Button::Button(u16 x, u16 y, u16 w, u16 h, std::string text, buttonGroup_t *buttonGroup) 
: m_x(x), m_y(y), m_w(w), m_h(h), m_text(text), m_buttonGroup(buttonGroup) {
  m_inputActions = [](u32 kdown){ };

  buttonGroup->buttons.push_back(this);
}

Button::~Button() {

}

void Button::draw(Gui *gui) {
  gui->drawShadow(m_x, m_y, m_w, m_h);

  if (m_buttonGroup->buttons[m_buttonGroup->selectedButton] == this) {
    gui->drawRectangle(m_x - 5, m_y - 5, m_w + 10, m_h + 12, currTheme.highlightColor);
    gui->drawShadow(m_x - 5, m_y - 5, m_w + 10, m_h + 10);
  }

  gui->drawRectangle(m_x, m_y, m_w, m_h, currTheme.selectedColor);

  u32 textHeight;
  gui->getTextDimensions(font20, m_text.c_str(), nullptr, &textHeight);
  gui->drawTextAligned(font20, m_x + (m_w / 2), m_y + (m_h / 2) - (textHeight / 2), currTheme.backgroundColor, m_text.c_str(), ALIGNED_CENTER);
}

void Button::onInput(u32 kdown) {

}

void Button::onTouch(touchPosition &touch) {

}

Button* Button::setInputAction(std::function<void(u32 kdown)> inputActions) {
  m_inputActions = inputActions;
  return this;
}
