#include "widgets/widget.hpp"

#include "guis/gui_editor.hpp"

#include <iostream>

static s32 tooltipCnt = 0;

Widget::Widget(Interpreter *interpreter, bool isDummy) : m_interpreter(interpreter), m_isDummy(isDummy) {
  Widget::g_stepSizeMultiplier = 1;
}

Widget::~Widget() {
  Widget::g_stepSizeMultiplier = 1;
}

void Widget::drawWidgets(Gui *gui, WidgetItems &widgets, u16 y, u16 start, u16 end) {
  static u16 oldWidgetIndex = 0;

  if (Widget::g_categories.empty() || widgets.empty()) return;
  if (widgets.find(Widget::g_selectedCategory) == widgets.end()) return;

  ptrdiff_t categoryIndex = std::find(Widget::g_categories.begin(), Widget::g_categories.end(), Widget::g_selectedCategory) - Widget::g_categories.begin() - g_categoryYOffset;
  if (Widget::g_selectedRow == CATEGORIES) {
    gui->drawRectangled(25, y + 4 + 60 * categoryIndex, 310, 55, currTheme.highlightColor);
    gui->drawRectangle(30, y + 9 + 60 * categoryIndex, 300, 45, currTheme.selectedButtonColor);
    gui->drawShadow(25, y + 4 + 60 * categoryIndex, 310, 55);
  }

  gui->drawRectangle(37, y + 13 + 60 * categoryIndex, 4, 35, currTheme.selectedColor);

  for (u8 i = 0; i < Widget::g_categories.size(); i++) {
    gui->drawText(font20, 50, y + 15 + 60 * (i - g_categoryYOffset), Widget::g_categories[i] == Widget::g_selectedCategory ? currTheme.selectedColor : currTheme.textColor, Widget::g_categories[i].c_str());
  }

  std::vector<WidgetItem> &currWidgets = widgets[Widget::g_selectedCategory];

  if (currWidgets.size() <= 0) return;

  u16 widgetInset = (Gui::g_framebuffer_width - WIDGET_WIDTH) / 2.0F;
  u16 widgetY = y;

  for (;start < end; start++) {
    if (start > currWidgets.size() - 1) break;

    if (start == Widget::g_selectedWidgetIndex && Widget::g_selectedRow == WIDGETS) {
      gui->drawRectangled(widgetInset + X_OFFSET, widgetY, WIDGET_WIDTH - 1, WIDGET_HEIGHT, currTheme.highlightColor);
      gui->drawRectangle(widgetInset + 5 + X_OFFSET, widgetY + 5, WIDGET_WIDTH - 12, WIDGET_HEIGHT - 10, currTheme.selectedButtonColor);
      gui->drawShadow(widgetInset + X_OFFSET, widgetY, WIDGET_WIDTH, WIDGET_HEIGHT);
    }

    gui->drawTextAligned(font20, widgetInset + 50 + X_OFFSET, widgetY + (WIDGET_HEIGHT / 2.0F) - 13, currTheme.textColor, currWidgets[start].title.c_str(), ALIGNED_LEFT);
    gui->drawRectangle(widgetInset + 30 + X_OFFSET, widgetY + WIDGET_HEIGHT + (WIDGET_SEPARATOR / 2) - 1, WIDGET_WIDTH - 60, 1, currTheme.separatorColor);
    currWidgets[start].widget->draw(gui, widgetInset + 50 + X_OFFSET, widgetY - 13);

    widgetY += WIDGET_HEIGHT + WIDGET_SEPARATOR;
  }

  if (Widget::g_selectedRow == WIDGETS) {
    if (Widget::g_selectedWidgetIndex != oldWidgetIndex)
      tooltipCnt = 0;
    else if (tooltipCnt < (3 * 60 + 0xFF))
      tooltipCnt = std::min(tooltipCnt + 8, 3 * 60 + 0xFF);

    if (Widget::g_selectedWidgetIndex % static_cast<u8>(WIDGETS_PER_PAGE) > 3)
      gui->drawTooltip(widgetInset + X_OFFSET + 200, y + (WIDGET_HEIGHT + WIDGET_SEPARATOR) * (Widget::g_selectedWidgetIndex % static_cast<u8>(WIDGETS_PER_PAGE)), "afasdfhasdkfhaskjdfhaksjdfhkasjdfhkjasdfhkjasdfhkasfdkjhsakdfhkasjd\n asdasdasdasdasd", currTheme.tooltipColor, currTheme.textColor, std::max(0, tooltipCnt - 3 * 60), true);
    else
      gui->drawTooltip(widgetInset + X_OFFSET + 200, y + (WIDGET_HEIGHT + WIDGET_SEPARATOR) * ((Widget::g_selectedWidgetIndex % static_cast<u8>(WIDGETS_PER_PAGE)) + 1), "afasdfhasdkfhaskjdfhaksjdfhkasjdfhkjasdfhkjasdfhkasfdkjhsakdfhkasjd\n asdasdasdasdasd", currTheme.tooltipColor, currTheme.textColor, std::max(0, tooltipCnt - 3 * 60), false);
  } else tooltipCnt = 0;

  oldWidgetIndex = Widget::g_selectedWidgetIndex;

}

void Widget::handleInput(u32 kdown, WidgetItems &widgets) {
  std::vector<WidgetItem> &currWidgets = widgets[Widget::g_selectedCategory];

  if (currWidgets.size() > 0 && Widget::g_selectedRow == WIDGETS)
    currWidgets[Widget::g_selectedWidgetIndex].widget->onInput(kdown);

  if (Widget::g_selectedRow == CATEGORIES) {
    if (kdown & KEY_A || kdown & KEY_RIGHT) {
      Widget::g_selectedRow = WIDGETS;
      Widget::g_selectedWidgetIndex = Widget::g_widgetPage * WIDGETS_PER_PAGE;
    }
  }

  if (kdown & KEY_RSTICK) {
    if (g_stepSizeMultiplier == 10000) g_stepSizeMultiplier = 1;
    else g_stepSizeMultiplier *= 10;
  }
}

s64 Widget::getIntegerValue() {
  m_interpreter->setArgs(m_intArgs, m_strArgs);

  return m_isDummy ? m_interpreter->getDummyValue() : m_interpreter->getValueFromSaveFile();
}

std::string Widget::getStringValue() {
  m_interpreter->setArgs(m_intArgs, m_strArgs);

  return m_isDummy ? m_interpreter->getDummyString() : m_interpreter->getStringFromSaveFile();
}

void Widget::setIntegerValue(s64 value) {
  m_interpreter->setArgs(m_intArgs, m_strArgs);

  if (Widget::isDummy()) 
    m_interpreter->setDummyValue(value);
  else m_interpreter->setValueInSaveFile(value);
}

void Widget::setStringValue(std::string value) {
  m_interpreter->setArgs(m_intArgs, m_strArgs);

  if (Widget::isDummy()) 
    m_interpreter->setDummyString(value);
  else m_interpreter->setStringInSaveFile(value);
}

void Widget::setLuaArgs(std::vector<s32> intArgs, std::vector<std::string> strArgs) {
  this->m_intArgs = intArgs;
  this->m_strArgs = strArgs;
}

bool Widget::isDummy() {
  return this->m_isDummy;
}
