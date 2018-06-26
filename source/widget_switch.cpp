#include "widget_switch.hpp"

WidgetSwitch::WidgetSwitch(u8 addressSize, u8 valueSize, u64 onValue, u64 offValue) : Widget(addressSize, valueSize), m_onValue(onValue), m_offValue(offValue) {

}

WidgetSwitch::~WidgetSwitch() {

}

void WidgetSwitch::draw(Gui *gui, u16 x, u16 y) {
  u32 textWidth, textHeight;
  gui->getTextDimensions(font20, Widget::getValue() == m_onValue ? "ON" : "OFF", &textWidth, &textHeight);

  gui->drawText(font20, x + ((WIDGET_WIDTH / 2.0F) - (textWidth / 2.0F)), y + ((WIDGET_HEIGHT / 2.0F) - (textHeight / 2.0F)),  Widget::getValue() ? currTheme.selectedColor : currTheme.separatorColor, Widget::getValue() == m_onValue ? "ON" : "OFF");
}

void WidgetSwitch::onInput(u32 kdown) {
  if (kdown & KEY_A) {
    if (Widget::getValue() == m_onValue)
      Widget::setValue(m_offValue);
    else
      Widget::setValue(m_onValue);
  }
}

void WidgetSwitch::onTouch(touchPosition &touch) {
  if (Widget::getValue() == m_offValue)
    Widget::setValue(m_onValue);
  else
    Widget::setValue(m_offValue);
}
