#include "widget.hpp"

Widget::Widget() {

}

Widget::~Widget() {

}

void Widget::drawWidgets(Gui *gui, WidgetList &widgets, u16 y, u16 start, u16 end) {
  for(auto widget : widgets) {
    gui->drawText(font20, 50, y, currTheme.textColor, widget.title.c_str());
    widget.widget->draw(gui, gui->framebuffer_width - WIDGET_WIDTH - 50, y);

    y += WIDGET_HEIGHT + WIDGET_SEPARATOR;
  }
}

u32 Widget::getValue() {
  return m_value;
}
