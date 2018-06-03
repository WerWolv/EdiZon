#include "widget_switch.hpp"

WidgetSwitch::WidgetSwitch() : Widget() {

}

WidgetSwitch::~WidgetSwitch() {

}

void WidgetSwitch::draw(Gui *gui, u16 x, u16 y) {
  u32 textWidth, textHeight;
  gui->getTextDimensions(font20, m_value == 0 ? "OFF" : "ON", &textWidth, &textHeight);

  gui->drawText(font20, x + ((WIDGET_WIDTH / 2.0F) - (textWidth / 2.0F)), y + ((WIDGET_HEIGHT / 2.0F) - (textHeight / 2.0F)), m_value ? currTheme.selectedColor : currTheme.separatorColor, m_value == 0 ? "OFF" : "ON");
}

void WidgetSwitch::onInput(u32 kdown) {
  if(kdown & KEY_A)
    m_value = !m_value;
}

void WidgetSwitch::onTouch(touchPosition &touch, u16 widgetX, u16 widgetY) {
  m_value = !m_value;
}
