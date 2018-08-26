#include "widget_value.hpp"

#include <math.h>

#define ACCELERATION_DELAY 50

WidgetValue::WidgetValue(LuaSaveParser *saveParser, std::string readEquation, std::string writeEquation, s64 minValue, s64 maxValue, u64 stepSize) :
 Widget(saveParser), m_readEquation(readEquation), m_writeEquation(writeEquation), m_minValue(minValue), m_maxValue(maxValue), m_stepSize(stepSize) {
  m_widgetDataType = INT;

  if (stepSize == 0)
    m_stepSize = floor((maxValue - minValue) / 500.0F);

  m_currValue = 0;

}

WidgetValue::~WidgetValue() {

}

void WidgetValue::draw(Gui *gui, u16 x, u16 y) {
  std::stringstream ss;
  ss << m_currValue;

  if (m_currValue == 0)
    m_currValue = Widget::m_saveParser->evaluateEquation(m_readEquation, Widget::getIntegerValue());

  gui->drawTextAligned(font20, x + WIDGET_WIDTH - 140, y + (WIDGET_HEIGHT / 2.0F), currTheme.selectedColor, ss.str().c_str(), ALIGNED_RIGHT);
}

void WidgetValue::onInput(u32 kdown) {
  static u32 accelerationTimer = 0;

  m_currValue = Widget::m_saveParser->evaluateEquation(m_readEquation, Widget::getIntegerValue());

  if (kdown & KEY_LEFT) {
    //accelerationTimer++;
    if (m_currValue > m_minValue) {
      /*if(accelerationTimer > ACCELERATION_DELAY && m_currValue > static_cast<s32>(m_minValue + m_stepSize))
        Widget::setIntegerValue(Widget::m_saveParser->evaluateEquation(m_writeEquation, m_currValue) - m_stepSize);
      else*/
        Widget::setIntegerValue(Widget::m_saveParser->evaluateEquation(m_writeEquation, m_currValue) - m_stepSize);
    }
    else Widget::setIntegerValue(Widget::m_saveParser->evaluateEquation(m_writeEquation, m_maxValue));
  }

  if (kdown & KEY_RIGHT) {
    //accelerationTimer++;
    if (m_currValue < m_maxValue) {
      /*if(accelerationTimer > 50 && m_currValue < static_cast<s32>(m_maxValue - m_stepSize))
        Widget::setIntegerValue(Widget::m_saveParser->evaluateEquation(m_writeEquation, m_currValue) + m_stepSize);
      else*/
        Widget::setIntegerValue(Widget::m_saveParser->evaluateEquation(m_writeEquation, m_currValue) + m_stepSize);
    }
    else Widget::setIntegerValue(Widget::m_saveParser->evaluateEquation(m_writeEquation, m_minValue));
  }

  /*if ((kdown & (KEY_LEFT | KEY_RIGHT)) == 0 ||
     ((kdown & KEY_RIGHT) == 0 && static_cast<s32>(Widget::m_saveParser->evaluateEquation(m_writeEquation, m_currValue) - m_stepSize) < m_minValue) ||
     ((kdown & KEY_LEFT) == 0 && static_cast<s32>(Widget::m_saveParser->evaluateEquation(m_writeEquation, m_currValue) + m_stepSize) > m_maxValue))
    accelerationTimer = 0;*/
}

void WidgetValue::onTouch(touchPosition &touch) {

}
