#include "widget.hpp"

#include "gui_editor.hpp"
#include <iostream>

u16 Widget::g_selectedWidgetIndex = 0;

Widget::Widget() {

}

Widget::~Widget() {

}

void Widget::drawWidgets(Gui *gui, WidgetList &widgets, u16 y, u16 start, u16 end) {
  if (widgets.size() <= 0) return;

  for (;start < end; start++) {
    if (start > widgets.size() - 1) break;

    if (start == Widget::g_selectedWidgetIndex) {
      gui->drawRectangled(150, y, gui->framebuffer_width - 300, WIDGET_HEIGHT, currTheme.highlightColor);
      gui->drawRectangle(155, y + 5, gui->framebuffer_width - 315, WIDGET_HEIGHT - 10, currTheme.selectedButtonColor);
      gui->drawShadow(150, y, gui->framebuffer_width - 300, WIDGET_HEIGHT);
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
  if (widgets.size() <= 0) return;
  widgets[Widget::g_selectedWidgetIndex].widget->onInput(kdown);
}

u16 Widget::getValue() {
  return getValueFromAddressAtOffset(&GuiEditor::g_currSaveFile, this->m_offsetAddress, this->m_address);
}

void Widget::setValue(u16 value) {
  setValueAtAddressAtOffset(&GuiEditor::g_currSaveFile, this->m_offsetAddress, this->m_address, value);
}

void Widget::setOffset(u16 offsetAddress, u16 address) {
  this->m_offsetAddress = offsetAddress;
  this->m_address = address;
}

void Widget::setOffset(u16 address) {
  this->m_offsetAddress = 0x0000;
  this->m_address = address;
}
