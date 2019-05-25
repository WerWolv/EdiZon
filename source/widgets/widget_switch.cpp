#include "widgets/widget_switch.hpp"

WidgetSwitch::WidgetSwitch(Interpreter *interpreter, bool isDummy, std::string tooltip, s32 onValue, s32 offValue) :
 Widget(interpreter, isDummy, tooltip), m_intOnValue(onValue), m_intOffValue(offValue) {
  m_widgetDataType = INT;
}

WidgetSwitch::WidgetSwitch(Interpreter *interpreter, bool isDummy, std::string tooltip, std::string onValue, std::string offValue) :
 Widget(interpreter, isDummy, tooltip), m_strOnValue(onValue), m_strOffValue(offValue){
  m_widgetDataType = STRING;
}


WidgetSwitch::~WidgetSwitch() {

}

void WidgetSwitch::draw(Gui *gui, u16 x, u16 y) {
  if (m_widgetDataType == INT) {
    s64 intValue = Widget::getIntegerValue();
    gui->drawTextAligned(font20, x + WIDGET_WIDTH - 140, y + (WIDGET_HEIGHT / 2.0F), intValue == m_intOnValue ? currTheme.selectedColor : currTheme.separatorColor, intValue == m_intOnValue ? "ON" : "OFF", ALIGNED_RIGHT);
  }
  else if (m_widgetDataType == STRING) {
    std::string strValue = Widget::getStringValue();
    gui->drawTextAligned(font20, x + WIDGET_WIDTH - 140, y + (WIDGET_HEIGHT / 2.0F), strValue == m_strOnValue ? currTheme.selectedColor : currTheme.separatorColor, strValue == m_strOnValue ? "ON" : "OFF", ALIGNED_RIGHT);
  }
}

void WidgetSwitch::onInput(u32 kdown) {
  if (kdown & KEY_A) {
    if (m_widgetDataType == INT) {
      if (Widget::getIntegerValue() == m_intOnValue)
        Widget::setIntegerValue(m_intOffValue);
      else
        Widget::setIntegerValue(m_intOnValue);
    } else if (m_widgetDataType == STRING) {
      if (Widget::getStringValue() == m_strOnValue)
        Widget::setStringValue(m_strOffValue);
      else
        Widget::setStringValue(m_strOnValue);
    }
  }
}

void WidgetSwitch::onTouch(touchPosition &touch) {
  if (m_widgetDataType == INT) {
    if (Widget::getIntegerValue() == m_intOnValue)
      Widget::setIntegerValue(m_intOffValue);
    else
      Widget::setIntegerValue(m_intOnValue);
  } else if (m_widgetDataType == STRING) {
    if (Widget::getStringValue() == m_strOnValue)
      Widget::setStringValue(m_strOffValue);
    else
      Widget::setStringValue(m_strOnValue);
  }
}
