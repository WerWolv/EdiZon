#pragma once

#include <switch.h>

#include <string>
#include <utility>

#include "gui.hpp"

#define WIDGET_WIDTH      300
#define WIDGET_HEIGHT     65
#define WIDGET_SEPARATOR  10

class Widget;
typedef struct { std::string title; Widget *widget; } WidgetPair;
typedef std::vector<WidgetPair> WidgetList;

class Widget {
public:
  static u16 g_selectedWidgetIndex;

  Widget();
  virtual ~Widget();

  static void drawWidgets(Gui *gui, WidgetList &widgets, u16 y, u16 start, u16 end);
  static void handleInput(u32 kdown, WidgetList &widgets);

  virtual void draw(Gui *gui, u16 x, u16 y) = 0;
  virtual void onInput(u32 kdown) = 0;
  virtual void onTouch(touchPosition &touch) = 0;

  u16 getValue();
  void setValue(u16 value);

  void setOffset(u16 offsetAddress, u16 address);
  void setOffset(u16 address);

private:
  u16 m_offsetAddress;
  u16 m_address;
};
