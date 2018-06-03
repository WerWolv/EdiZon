#include "widget.hpp"

u16 Widget::g_selectedWidgetIndex = 0;

Widget::Widget() {

}

Widget::~Widget() {

}

void Widget::drawWidgets(Gui *gui, WidgetList &widgets, u16 y, u16 start, u16 end) {
  for(;start < end; start++) {
    if(start > widgets.size() - 1) break;

    if(start == Widget::g_selectedWidgetIndex) {
      gui->drawRectangled(150, y, gui->framebuffer_width - 300, WIDGET_HEIGHT, currTheme.highlightColor);
      gui->drawRectangle(155, y + 5, gui->framebuffer_width - 315, WIDGET_HEIGHT - 10, currTheme.selectedButtonColor);
      gui->drawShadow(155, y + 5, gui->framebuffer_width - 315, WIDGET_HEIGHT - 10);
    }

    u32 textWidth, textHeight;
    gui->getTextDimensions(font20, widgets[start].title.c_str(), &textWidth, &textHeight);
    gui->drawText(font20, 200, y + ((WIDGET_HEIGHT / 2.0F) - (textHeight / 2.0F)) - 13, currTheme.textColor, widgets[start].title.c_str());
    gui->drawRectangle(50, y + WIDGET_HEIGHT + (WIDGET_SEPARATOR / 2) - 1, gui->framebuffer_width - 100, 1, currTheme.separatorColor);
    widgets[start].widget->draw(gui, gui->framebuffer_width - WIDGET_WIDTH - 100, y - 13);

    y += WIDGET_HEIGHT + WIDGET_SEPARATOR;
  }
}

void Widget::handleInput(u32 kdown, WidgetList &widgets) {
  widgets[Widget::g_selectedWidgetIndex].widget->onInput(kdown);
}

u16 Widget::getValue() {
  return m_value;
}

void Widget::setValue(u16 value) {
  this->m_value = value;
}
