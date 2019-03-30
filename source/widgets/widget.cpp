#include "widgets/widget.hpp"

#include "guis/gui_editor.hpp"

#include <iostream>

static s32 tooltipCnt = 0;

Widget::Widget(Interpreter *interpreter, bool isDummy, std::string tooltip) : m_interpreter(interpreter), m_isDummy(isDummy), m_tooltip(tooltip) {
  Widget::g_stepSizeMultiplier = 1;
}

Widget::~Widget() {
  Widget::g_stepSizeMultiplier = 1;
}

void Widget::drawWidgets(Gui *gui, WidgetItems &widgets, u16 start, u16 end) {
  static u16 oldWidgetIndex = 0;

  if (Widget::g_categories.empty() || widgets.empty()) return;
  if (widgets.find(Widget::g_selectedCategory) == widgets.end()) return;

  ptrdiff_t categoryIndex = std::find(Widget::g_categories.begin(), Widget::g_categories.end(), Widget::g_selectedCategory) - Widget::g_categories.begin() - g_categoryYOffset;
  if (Widget::g_selectedRow == CATEGORIES) {
    gui->drawRectangled(25, 150 + 4 + 60 * categoryIndex, 310, 55, currTheme.highlightColor);
    gui->drawRectangle(30, 150 + 9 + 60 * categoryIndex, 300, 45, currTheme.selectedButtonColor);
    gui->drawShadow(25, 150 + 4 + 60 * categoryIndex, 310, 55);
  }

  gui->drawRectangle(37, 150 + 13 + 60 * categoryIndex, 4, 35, currTheme.selectedColor);

  for (u8 i = 0; i < Widget::g_categories.size(); i++) {
    gui->drawText(font20, 50, 150 + 15 + 60 * (i - g_categoryYOffset), Widget::g_categories[i] == Widget::g_selectedCategory ? currTheme.selectedColor : currTheme.textColor, Widget::g_categories[i].c_str());
  }

  std::vector<WidgetItem> &currWidgets = widgets[Widget::g_selectedCategory];

  if (currWidgets.size() <= 0) return;

  u16 widgetInset = (Gui::g_framebuffer_width - WIDGET_WIDTH) / 2.0F;
  u16 widgetY = 150;

  for (;start < end; start++) {
    if (start > currWidgets.size() - 1) break;

    if (start == Widget::g_selectedWidgetIndex && Widget::g_selectedRow == WIDGETS) {
      gui->drawRectangled(widgetInset + X_OFFSET, widgetY, WIDGET_WIDTH - 1, WIDGET_HEIGHT, currTheme.highlightColor);
      gui->drawRectangle(widgetInset + 5 + X_OFFSET, widgetY + 5, WIDGET_WIDTH - 12, WIDGET_HEIGHT - 10, currTheme.selectedButtonColor);
      gui->drawShadow(widgetInset + X_OFFSET, widgetY, WIDGET_WIDTH, WIDGET_HEIGHT);
    }

    gui->drawTextAligned(font20, widgetInset + 50 + X_OFFSET, widgetY + (WIDGET_HEIGHT / 2.0F) - 13, currTheme.textColor, currWidgets[start].title.c_str(), ALIGNED_LEFT);
    gui->drawRectangle(widgetInset + 30 + X_OFFSET, widgetY + WIDGET_HEIGHT + (WIDGET_SEPARATOR / 2) - 1, WIDGET_WIDTH - 60, 1, gui->makeColor(0x00, 0x00, 0x00, 0xFF));
    currWidgets[start].widget->draw(gui, widgetInset + 50 + X_OFFSET, widgetY - 13);

    widgetY += WIDGET_HEIGHT + WIDGET_SEPARATOR;
  }

  if (Widget::g_selectedRow == WIDGETS) {
    if (Widget::g_selectedWidgetIndex != oldWidgetIndex)
      tooltipCnt = 0;
    else if (tooltipCnt < (8 * 60 + 0xFF))
      tooltipCnt = std::min(tooltipCnt + 16, 8 * 60 + 0xFF);

    if (currWidgets[Widget::g_selectedWidgetIndex].widget->m_tooltip != "") {
      if (Widget::g_selectedWidgetIndex % static_cast<u8>(WIDGETS_PER_PAGE) > 3)
        gui->drawTooltip(widgetInset + X_OFFSET + 200, 150 + (WIDGET_HEIGHT + WIDGET_SEPARATOR) * (Widget::g_selectedWidgetIndex % static_cast<u8>(WIDGETS_PER_PAGE)), currWidgets[Widget::g_selectedWidgetIndex].widget->m_tooltip.c_str(), currTheme.tooltipColor, currTheme.textColor, std::max(0, tooltipCnt - 8 * 60), true);
      else
        gui->drawTooltip(widgetInset + X_OFFSET + 200, 150 + (WIDGET_HEIGHT + WIDGET_SEPARATOR) * ((Widget::g_selectedWidgetIndex % static_cast<u8>(WIDGETS_PER_PAGE)) + 1), currWidgets[Widget::g_selectedWidgetIndex].widget->m_tooltip.c_str(), currTheme.tooltipColor, currTheme.textColor, std::max(0, tooltipCnt - 8 * 60), false);   
    }
  } else tooltipCnt = 0;

  oldWidgetIndex = Widget::g_selectedWidgetIndex;

}

void Widget::handleInput(u32 kdown, WidgetItems &widgets) {
  std::vector<WidgetItem> &currWidgets = widgets[Widget::g_selectedCategory];

  if (currWidgets.size() > 0 && Widget::g_selectedRow == WIDGETS)
    currWidgets[Widget::g_selectedWidgetIndex].widget->onInput(kdown);

  if (Widget::g_selectedRow == CATEGORIES) {
    if (kdown & KEY_A || kdown & KEY_LSTICK_RIGHT || kdown & KEY_RSTICK_RIGHT) {
      Widget::g_selectedRow = WIDGETS;
      Widget::g_selectedWidgetIndex = Widget::g_widgetPage * WIDGETS_PER_PAGE;
    }
  }

  if (kdown & KEY_RSTICK) {
    if (g_stepSizeMultiplier == 10000) g_stepSizeMultiplier = 1;
    else g_stepSizeMultiplier *= 10;
  }
}

void Widget::handleTouch(touchPosition &touch, WidgetItems &widgets) {
  std::vector<WidgetItem> &currWidgets = widgets[Widget::g_selectedCategory];
  
  u16 widgetInset = (Gui::g_framebuffer_width - WIDGET_WIDTH) / 2.0F;

  if (touch.px > 30 && touch.px < 330) { /* Touch of categories area */
    u8 categoryIndex = std::floor((touch.py - 159) / 60.0F) + g_categoryYOffset;

    Widget::g_selectedRow = CATEGORIES;

    if (Widget::g_selectedCategory == Widget::g_categories[categoryIndex])
      Widget::g_selectedRow = WIDGETS;
    else if (categoryIndex < Widget::g_categories.size()) {
      Widget::g_selectedCategory = Widget::g_categories[categoryIndex];
      Widget::g_selectedWidgetIndex = categoryIndex;
    }

    if (Widget::g_selectedWidgetIndex < Widget::g_categories.size() - 7 && Widget::g_categoryYOffset != 0)
      Widget::g_categoryYOffset--;
    
    if (Widget::g_selectedWidgetIndex > 6 && Widget::g_categoryYOffset < Widget::g_categories.size() - 8)
      Widget::g_categoryYOffset++;

  } else if (touch.px > widgetInset + X_OFFSET && touch.px < widgetInset + X_OFFSET + WIDGET_WIDTH
            && touch.py > 150 && touch.py < 150 + (WIDGET_HEIGHT + WIDGET_SEPARATOR) * WIDGETS_PER_PAGE) { /* Touch of widgets area */
    u16 widgetIndex = std::min(static_cast<u32>((touch.py - 150) / (WIDGET_HEIGHT + WIDGET_SEPARATOR) + Widget::g_widgetPage * WIDGETS_PER_PAGE), static_cast<u32>(currWidgets.size()));
    
    Widget::g_selectedRow = WIDGETS;

    if (Widget::g_selectedWidgetIndex != widgetIndex)
      Widget::g_selectedWidgetIndex = widgetIndex;
    else currWidgets[widgetIndex].widget->onTouch(touch);
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
