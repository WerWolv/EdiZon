#include "widget.hpp"

Widget::Widget() {

}

Widget::~Widget() {

}

void Widget::drawWidgets(Gui *gui, WidgetList &widgets, u16 y, u16 start, u16 end) {
  for(auto widget : widgets) {
    u32 textWidth, textHeight;
    gui->getTextDimensions(font20, widget.title.c_str(), &textWidth, &textHeight);
    gui->drawText(font20, 100, y + ((WIDGET_HEIGHT / 2.0F) - (textHeight / 2.0F)), currTheme.textColor, widget.title.c_str());
    gui->drawRectangle(50, y + WIDGET_HEIGHT + (WIDGET_SEPARATOR / 2) - 1, gui->framebuffer_width - 100, 2, currTheme.textColor);
    widget.widget->draw(gui, gui->framebuffer_width - WIDGET_WIDTH - 100, y);

    y += WIDGET_HEIGHT + WIDGET_SEPARATOR;
  }
}

u32 Widget::getValue() {
  return m_value;
}
