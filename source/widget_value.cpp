#include "widget_value.hpp"

#define ACCELERATED_SPEED 20
#define ACCELERATION_DELAY 50

WidgetValue::WidgetValue(LuaSaveParser *saveParser, s32 minValue, s32 maxValue) : Widget(saveParser), m_minValue(minValue), m_maxValue(maxValue) {
  m_widgetDataType = INT;
}

WidgetValue::~WidgetValue() {

}

void WidgetValue::draw(Gui *gui, u16 x, u16 y) {
  std::stringstream ss;
  ss << Widget::getIntegerValue();

  gui->drawTextAligned(font20, x + WIDGET_WIDTH - 140, y + (WIDGET_HEIGHT / 2.0F), currTheme.selectedColor, ss.str().c_str(), ALIGNED_RIGHT);
}

void WidgetValue::onInput(u32 kdown) {
  static u32 accelerationTimer = 0;

  if (kdown & KEY_LEFT) {
    accelerationTimer++;
    if (Widget::getIntegerValue() > m_minValue) {
      if(accelerationTimer > ACCELERATION_DELAY && Widget::getIntegerValue() > m_minValue + ACCELERATED_SPEED)
        Widget::setIntegerValue(Widget::getIntegerValue() - ACCELERATED_SPEED);
      else
        Widget::setIntegerValue(Widget::getIntegerValue() - 1);
    }
    else Widget::setIntegerValue(m_maxValue);
  }

  if (kdown & KEY_RIGHT) {
    accelerationTimer++;
    if (Widget::getIntegerValue() < m_maxValue) {
      if(accelerationTimer > 50 && Widget::getIntegerValue() < m_maxValue - ACCELERATED_SPEED)
        Widget::setIntegerValue(Widget::getIntegerValue() + ACCELERATED_SPEED);
      else
        Widget::setIntegerValue(Widget::getIntegerValue() + 1);
    }
    else Widget::setIntegerValue(m_minValue);
  }

  if ((kdown & (KEY_LEFT | KEY_RIGHT)) == 0 ||
      Widget::getIntegerValue() - ACCELERATED_SPEED <= m_minValue || Widget::getIntegerValue() + ACCELERATED_SPEED >= m_maxValue)
    accelerationTimer = 0;
}

void WidgetValue::onTouch(touchPosition &touch) {

}
