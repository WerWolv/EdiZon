#include "widget_value.hpp"

WidgetValue::WidgetValue(u8 addressSize, u8 valueSize, u64 minValue, u64 maxValue) : Widget(addressSize, valueSize), m_minValue(minValue), m_maxValue(maxValue) {
}

WidgetValue::~WidgetValue() {

}

void WidgetValue::draw(Gui *gui, u16 x, u16 y) {
  std::stringstream ss;
  ss << Widget::getValue() << " [0x" << std::setfill('0') << std::setw(Widget::m_valueSize * 2) << std::uppercase << std::hex << Widget::getValue() << "]";

  gui->drawTextAligned(font20, x + WIDGET_WIDTH - 140, y + (WIDGET_HEIGHT / 2.0F), currTheme.selectedColor, ss.str().c_str(), ALIGNED_RIGHT);
}

void WidgetValue::onInput(u32 kdown) {
  if (kdown & KEY_LEFT) {
    if (Widget::getValue() > m_minValue)
      Widget::setValue(Widget::getValue() - 1);
    else Widget::setValue(m_maxValue);
  }

  if (kdown & KEY_RIGHT) {
    if (Widget::getValue() < m_maxValue)
      Widget::setValue(Widget::getValue() + 1);
    else Widget::setValue(m_minValue);
  }
}

void WidgetValue::onTouch(touchPosition &touch) {

}
