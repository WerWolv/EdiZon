#pragma once

#include <string>
#include <utility>

#include "gui.hpp"

#define WIDGET_WIDTH      300
#define WIDGET_HEIGHT     75
#define WIDGET_SEPARATOR  20

class Widget;
typedef struct { std::string title; Widget *widget; } WidgetPair;
typedef std::vector<WidgetPair> WidgetList;

class Widget {
public:
  static u16 g_selectedWidgetIndex;

  Widget();
  virtual ~Widget();

  virtual void draw(Gui *gui, u16 x, u16 y) = 0;
  static void drawWidgets(Gui *gui, WidgetList &widgets, u16 y, u16 start, u16 end);

  virtual void onInput(u32 kdown) = 0;
  virtual void onTouch(touchPosition &touch, u16 widgetX, u16 widgetY) = 0;

  u32 getValue();

protected:
  u32 m_value;
};
