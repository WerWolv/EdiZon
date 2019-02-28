#include "widgets/widget_value.hpp"

#include <math.h>

#define ACCELERATION_DELAY 50

WidgetValue::WidgetValue(Interpreter *interpreter, bool isDummy, std::string tooltip, std::string readEquation, std::string writeEquation, s64 minValue, s64 maxValue, u64 stepSize) :
 Widget(interpreter, isDummy, tooltip), m_readEquation(readEquation), m_writeEquation(writeEquation), m_minValue(minValue), m_maxValue(maxValue), m_stepSize(stepSize) {
  m_widgetDataType = INT;

  m_currValue = 0;

}

WidgetValue::~WidgetValue() {

}

void WidgetValue::draw(Gui *gui, u16 x, u16 y) {
  std::stringstream ss;
  ss << m_currValue;

  m_currValue = Widget::m_interpreter->evaluateEquation(m_readEquation, Widget::getIntegerValue());

  gui->drawTextAligned(font20, x + WIDGET_WIDTH - 140, y + (WIDGET_HEIGHT / 2.0F), currTheme.selectedColor, ss.str().c_str(), ALIGNED_RIGHT);
}

bool isNumber(const std::string &line) {
 if (line[0] == '0') return true;
 return (atoi(line.c_str()));
}

void WidgetValue::onInput(u32 kdown) {
  u64 incrementValue = m_stepSize * g_stepSizeMultiplier;

  if (kdown & KEY_DLEFT) {
    if (static_cast<s64>(m_currValue - incrementValue) >= m_minValue)
        Widget::setIntegerValue(Widget::m_interpreter->evaluateEquation(m_writeEquation, m_currValue) - incrementValue);
    else if(m_currValue < m_minValue)
      Widget::setIntegerValue(Widget::m_interpreter->evaluateEquation(m_writeEquation, m_maxValue));
    else
      Widget::setIntegerValue(Widget::m_interpreter->evaluateEquation(m_writeEquation, m_minValue));
  }

  if (kdown & KEY_DRIGHT) {
    if (static_cast<s64>(m_currValue + incrementValue) <= m_maxValue)
      Widget::setIntegerValue(Widget::m_interpreter->evaluateEquation(m_writeEquation, m_currValue) + incrementValue);
    else if(m_currValue > m_maxValue)
      Widget::setIntegerValue(Widget::m_interpreter->evaluateEquation(m_writeEquation, m_minValue));
    else
      Widget::setIntegerValue(Widget::m_interpreter->evaluateEquation(m_writeEquation, m_maxValue));
  }

  if (kdown & KEY_A) {
    u8 maxDigits = static_cast<u8>(std::floor(std::log10(m_maxValue)) + 1);

    char out_number[maxDigits + 1];
    Gui::requestKeyboardInput("Input value", "Enter a number for this value to be set to.", std::to_string(m_currValue).c_str(), SwkbdType_NumPad, out_number, maxDigits);

    if (isNumber(std::string(out_number)))
      Widget::setIntegerValue(
        Widget::m_interpreter->evaluateEquation(m_writeEquation, 
        Widget::m_interpreter->evaluateEquation(m_readEquation, 
        std::min(std::max(static_cast<s64>(atoi(out_number)), 
        m_minValue), m_maxValue))));
  }
  
  m_currValue = Widget::m_interpreter->evaluateEquation(m_readEquation, Widget::getIntegerValue());
}

void WidgetValue::onTouch(touchPosition &touch) {
  u8 maxDigits = static_cast<u8>(std::floor(std::log10(m_maxValue)) + 1);

  char out_number[maxDigits + 1];
  Gui::requestKeyboardInput("Input value", "Enter a number for this value to be set to.", std::to_string(m_currValue).c_str(), SwkbdType_NumPad, out_number, maxDigits);

  if (isNumber(std::string(out_number)))
    Widget::setIntegerValue(
      Widget::m_interpreter->evaluateEquation(m_writeEquation, 
      Widget::m_interpreter->evaluateEquation(m_readEquation, 
      std::min(std::max(static_cast<s64>(atoi(out_number)), 
      m_minValue), m_maxValue))));

  m_currValue = Widget::m_interpreter->evaluateEquation(m_readEquation, Widget::getIntegerValue());
}
