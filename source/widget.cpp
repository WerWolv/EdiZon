#include "widget.hpp"

#include "gui_editor.hpp"
#include <iostream>

Widget::Widget(LuaSaveParser *saveParser) : m_saveParser(saveParser) {

}

Widget::~Widget() {

}

void Widget::drawWidgets(Gui *gui, WidgetItems &widgets, u16 y, u16 start, u16 end) {

  u8 categoryIndex = 0;
  for (auto category : Widget::g_categories) {
    if (category == Widget::g_selectedCategory) {
      if (Widget::g_selectedRow == CATEGORIES) {
        gui->drawRectangled(25, y + 4 + 60 * categoryIndex, 310, 55, currTheme.highlightColor);
        gui->drawRectangle(30, y + 9 + 60 * categoryIndex, 300, 45, currTheme.selectedButtonColor);
        gui->drawShadow(25, y + 4 + 60 * categoryIndex, 310, 55);
      }

      gui->drawRectangle(37, y + 13 + 60 * categoryIndex, 4, 35, currTheme.selectedColor);
    }

    gui->drawText(font20, 50, y + 15 + 60 * categoryIndex, category == Widget::g_selectedCategory ? currTheme.selectedColor : currTheme.textColor, category.c_str());

    categoryIndex++;
  }


  std::vector<WidgetItem> &currWidgets = widgets[Widget::g_selectedCategory];

  if (currWidgets.size() <= 0) return;

  u16 widgetInset = (Gui::g_framebuffer_width - WIDGET_WIDTH) / 2.0F;

  for (;start < end; start++) {
    if (start > currWidgets.size() - 1) break;

    if (start == Widget::g_selectedWidgetIndex && Widget::g_selectedRow == WIDGETS) {
      gui->drawRectangled(widgetInset + X_OFFSET, y, WIDGET_WIDTH, WIDGET_HEIGHT, currTheme.highlightColor);
      gui->drawRectangle(widgetInset + 5 + X_OFFSET, y + 5, WIDGET_WIDTH - 12, WIDGET_HEIGHT - 10, currTheme.selectedButtonColor);
      gui->drawShadow(widgetInset + X_OFFSET, y, WIDGET_WIDTH, WIDGET_HEIGHT);
    }

    u32 textWidth, textHeight;
    gui->getTextDimensions(font20, currWidgets[start].title.c_str(), &textWidth, &textHeight);
    gui->drawTextAligned(font20, widgetInset + 50 + X_OFFSET, y + (WIDGET_HEIGHT / 2.0F) - 13, currTheme.textColor, currWidgets[start].title.c_str(), ALIGNED_LEFT);
    gui->drawRectangle(widgetInset + 30 + X_OFFSET, y + WIDGET_HEIGHT + (WIDGET_SEPARATOR / 2) - 1, WIDGET_WIDTH - 60, 1, currTheme.separatorColor);
    currWidgets[start].widget->draw(gui, widgetInset + 50 + X_OFFSET, y - 13);

    y += WIDGET_HEIGHT + WIDGET_SEPARATOR;
  }
}

void Widget::handleInput(u32 kdown, WidgetItems &widgets) {
  std::vector<WidgetItem> &currWidgets = widgets[Widget::g_selectedCategory];

  if (currWidgets.size() > 0 && Widget::g_selectedRow == WIDGETS)
    currWidgets[Widget::g_selectedWidgetIndex].widget->onInput(kdown);

  if (Widget::g_selectedRow == CATEGORIES) {
    if (kdown & KEY_A || kdown & KEY_RIGHT) {
      Widget::g_selectedRow = WIDGETS;
      Widget::g_selectedWidgetIndex = 0;
    }
  }
}

s32 Widget::getIntegerValue() {
  m_saveParser->setLuaArgs(m_intArgs, m_strArgs);
  return m_saveParser->getValueFromSaveFile();
}

std::string Widget::getStringValue() {
  m_saveParser->setLuaArgs(m_intArgs, m_strArgs);
  return m_saveParser->getStringFromSaveFile();
}

void Widget::setIntegerValue(s32 value) {
  m_saveParser->setLuaArgs(m_intArgs, m_strArgs);
  m_saveParser->setValueInSaveFile(value);
}

void Widget::setStringValue(std::string value) {
  m_saveParser->setLuaArgs(m_intArgs, m_strArgs);
  m_saveParser->setStringInSaveFile(value);
}

void Widget::setLuaArgs(std::vector<s32> intArgs, std::vector<std::string> strArgs) {
  this->m_intArgs = intArgs;
  this->m_strArgs = strArgs;
}
