#include "widgets/widget_comment.hpp"

WidgetComment::WidgetComment(Interpreter *interpreter, std::string tooltip, std::string text) :
 Widget(interpreter, true, tooltip), m_text(text) {
  m_widgetDataType = STRING;
}

WidgetComment::~WidgetComment() {

}

void WidgetComment::draw(Gui *gui, u16 x, u16 y) {
  std::string displayString = m_text;

  std::replace(displayString.begin(), displayString.end(), '\n', ' ');

  if(displayString.length() >= 25) {
    displayString = displayString.substr(0, 24);
    displayString += "...";
  }

    gui->drawTextAligned(font20, x + WIDGET_WIDTH - 140, y + (WIDGET_HEIGHT / 2.0F), gui->makeColor(0x00, 0x00, 0x00, 0xFF), displayString.c_str(), ALIGNED_RIGHT);
}

void WidgetComment::onInput(u32 kdown) {
  if (kdown & KEY_A) 
    (new MessageBox(m_text, MessageBox::OKAY))->show();
}

void WidgetComment::onTouch(touchPosition &touch) {
  (new MessageBox(m_text, MessageBox::OKAY))->show();
}
