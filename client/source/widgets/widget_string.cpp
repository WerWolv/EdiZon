#include "widgets/widget_string.hpp"

WidgetString::WidgetString(Interpreter *interpreter, bool isDummy, std::string tooltip, u8 minLength, u8 maxLength) :
 Widget(interpreter, isDummy, tooltip), m_minLength(minLength), m_maxLength(maxLength) {
  m_widgetDataType = STRING;
}

WidgetString::~WidgetString() {

}

void WidgetString::draw(Gui *gui, u16 x, u16 y) {
  std::string displayString = Widget::getStringValue();

  if(displayString.length() >= 15) {
    displayString = displayString.substr(0, 14);
    displayString += "...";
  }

  gui->drawTextAligned(font20, x + WIDGET_WIDTH - 140, y + (WIDGET_HEIGHT / 2.0F), currTheme.selectedColor, displayString.c_str(), ALIGNED_RIGHT);
}

void WidgetString::onInput(u32 kdown) {
  if (kdown & KEY_A) {
    char out_string[m_maxLength + 1];
    Gui::requestKeyboardInput("Input string value", "Enter a string for this value to be set to.", Widget::getStringValue(), SwkbdType_Normal, out_string, m_maxLength);

    if (std::strlen(out_string) > m_minLength)
      Widget::setStringValue(out_string);
    else (new Snackbar("Input string has to be longer than " + std::to_string(m_minLength) + " characters!"))->show();
  }
}

void WidgetString::onTouch(touchPosition &touch) {
  char out_string[m_maxLength + 1];
  Gui::requestKeyboardInput("Input string value", "Enter a string for this value to be set to.", Widget::getStringValue(), SwkbdType_Normal, out_string, m_maxLength);

  if (std::strlen(out_string) > m_minLength)
    Widget::setStringValue(out_string);
  else (new Snackbar("Input string has to be longer than " + std::to_string(m_minLength) + " characters!"))->show();
}
