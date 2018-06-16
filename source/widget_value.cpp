#include "widget_value.hpp"
#include "save.hpp"

WidgetValue::WidgetValue(u16 minValue, u16 maxValue, u16 offsetAddr, u16 address, u8* buffer) : Widget(), m_minValue(minValue), m_maxValue(maxValue) {
  m_address = *((u16*)(buffer + offsetAddr));
  if (buffer == NULL)
    m_value = 0;
  else
    m_value = getValueFromAddress(buffer, address);
  printf("offsetaddr: %d, address: %d, m_value: %d\n", offsetAddr, m_address, m_value);
  m_buffer = &buffer;
}

WidgetValue::~WidgetValue() {
  //setValueAtAddress(*m_buffer, m_address, m_value);
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
