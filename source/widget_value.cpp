#include "widget_value.hpp"

WidgetValue::WidgetValue(u16 minValue, u16 maxValue) : m_minValue(minValue), m_maxValue(maxValue), Widget() {
  m_value = 0;
}

WidgetValue::~WidgetValue() {

}

void WidgetValue::draw(Gui *gui, u16 x, u16 y) {
  u32 textWidth, textHeight;
  std::stringstream ss;
  ss << std::setfill('0') << std::setw(5) << m_value << " [0x" << std::setfill('0') << std::setw(4) << std::uppercase << std::hex << m_value << "]";

  gui->getTextDimensions(font20, ss.str().c_str(), &textWidth, &textHeight);

  gui->drawText(font20, x + (WIDGET_WIDTH / 2.0F) - 150, y + (WIDGET_HEIGHT / 2.0F) - (textHeight / 2.0F), currTheme.selectedColor, ss.str().c_str());
}

void WidgetValue::onInput(u32 kdown) {
  if(kdown & KEY_LEFT) {
    if(m_value > m_minValue)
      m_value--;
    else m_value = m_maxValue;
  }

  if(kdown & KEY_RIGHT) {
    if(m_value <= m_maxValue)
      m_value++;
    else m_value = m_minValue;
  }
}

void WidgetValue::onTouch(touchPosition &touch) {

}
