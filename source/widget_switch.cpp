#include "widget_switch.hpp"
#include "save.hpp"

WidgetSwitch::WidgetSwitch(u16 onValue, u16 offValue, u16 offsetAddr, u16 address, u8* buffer) : Widget(), m_onValue(onValue), m_offValue(offValue) {
  m_address = *((u16*)(buffer + offsetAddr));
  if (buffer == NULL)
    m_value = 0;
  else
    m_value = getValueFromAddress(buffer, address);
  m_buffer = &buffer;
}

WidgetSwitch::~WidgetSwitch() {
  setValueAtAddress(*m_buffer, m_address, m_value);
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

void WidgetSwitch::onTouch(touchPosition &touch) {
  m_value = !m_value;
}
