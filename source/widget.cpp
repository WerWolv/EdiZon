#include "widget.hpp"

#include "gui_editor.hpp"
#include <iostream>

u16 Widget::g_selectedWidgetIndex = 0;

Widget::Widget(LuaSaveParser *saveParser) : m_saveParser(saveParser) {

}

Widget::~Widget() {

}

void Widget::drawWidgets(Gui *gui, WidgetItems &widgets, u16 y, u16 start, u16 end) {
  if (widgets.size() <= 0) return;

  for (;start < end; start++) {
    if (start > widgets.size() - 1) break;

    if (start == Widget::g_selectedWidgetIndex) {
      gui->drawRectangled(150, y, Gui::g_framebuffer_width - 300, WIDGET_HEIGHT, currTheme.highlightColor);
      gui->drawRectangle(155, y + 5, Gui::g_framebuffer_width - 315, WIDGET_HEIGHT - 10, currTheme.selectedButtonColor);
      gui->drawShadow(150, y, Gui::g_framebuffer_width - 300, WIDGET_HEIGHT);
    }

    u32 textWidth, textHeight;
    gui->getTextDimensions(font20, widgets[start].title.c_str(), &textWidth, &textHeight);
    gui->drawText(font20, 200, y + ((WIDGET_HEIGHT / 2.0F) - (textHeight / 2.0F)) - 13, currTheme.textColor, widgets[start].title.c_str());
    gui->drawRectangle(100, y + WIDGET_HEIGHT + (WIDGET_SEPARATOR / 2) - 1, Gui::g_framebuffer_width - 200, 1, currTheme.separatorColor);
    widgets[start].widget->draw(gui, Gui::g_framebuffer_width - WIDGET_WIDTH - 100, y - 13);

    y += WIDGET_HEIGHT + WIDGET_SEPARATOR;
  }
}

void Widget::handleInput(u32 kdown, WidgetItems &widgets) {
  if (widgets.size() > 0)
    widgets[Widget::g_selectedWidgetIndex].widget->onInput(kdown);
}

u64 Widget::getIntegerValue() {
  m_saveParser->setLuaArgs(m_intArgs, m_strArgs);
  return m_saveParser->getValueFromSaveFile();
}

std::string Widget::getStringValue() {
  m_saveParser->setLuaArgs(m_intArgs, m_strArgs);
  return m_saveParser->getStringFromSaveFile();
}

void Widget::setIntegerValue(u64 value) {
  m_saveParser->setLuaArgs(m_intArgs, m_strArgs);
  m_saveParser->setValueInSaveFile(value);
}

void Widget::setStringValue(std::string value) {
  m_saveParser->setLuaArgs(m_intArgs, m_strArgs);
  m_saveParser->setStringInSaveFile(value);
}

void Widget::setLuaArgs(std::vector<u64> intArgs, std::vector<std::string> strArgs) {
  this->m_intArgs = intArgs;
  this->m_strArgs = strArgs;
}
