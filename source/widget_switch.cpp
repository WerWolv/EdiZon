#include "widget_switch.hpp"

WidgetSwitch::WidgetSwitch() : Widget() {

}

WidgetSwitch::~WidgetSwitch() {
  
}

void WidgetSwitch::draw(Gui *gui, u16 x, u16 y) {
  gui->drawRectangle(x, y, WIDGET_WIDTH, WIDGET_HEIGHT, currTheme.textColor);
}

void WidgetSwitch::onInput(u32 kdown) {

}

void WidgetSwitch::onTouch(touchPosition &touch, u16 widgetX, u16 widgetY) {

}
