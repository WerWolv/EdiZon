#include "widget_switch.hpp"

WidgetSwitch::WidgetSwitch(u16 onValue, u16 offValue) : Widget(), m_onValue(onValue), m_offValue(offValue) {

}

/*WidgetSwitch::WidgetSwitch(std::string onValue, std::string offValue) : Widget(), m_onValueStr(onValue), m_offValueStr(offValue) {

}*/

WidgetSwitch::~WidgetSwitch() {

}

void WidgetSwitch::draw(Gui *gui, u16 x, u16 y) {
  u32 textWidth, textHeight;
  gui->getTextDimensions(font20, Widget::getValue() == m_offValue ? "OFF" : "ON", &textWidth, &textHeight);

  gui->drawText(font20, x + ((WIDGET_WIDTH / 2.0F) - (textWidth / 2.0F)), y + ((WIDGET_HEIGHT / 2.0F) - (textHeight / 2.0F)),  Widget::getValue() ? currTheme.selectedColor : currTheme.separatorColor, Widget::getValue() == m_offValue ? "OFF" : "ON");
}

void WidgetSwitch::onInput(u32 kdown) {
  if(kdown & KEY_A) {
    if(Widget::getValue() == m_offValue)
      Widget::setValue(m_onValue);
    else
      Widget::setValue(m_offValue);
  }
}

void WidgetSwitch::onTouch(touchPosition &touch) {
  if(Widget::getValue() == m_offValue)
    Widget::setValue(m_onValue);
  else
    Widget::setValue(m_offValue);
}
