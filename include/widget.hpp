#pragma once

#include <switch.h>

#include <string>
#include <utility>

#include "gui.hpp"

#define WIDGET_WIDTH      300
#define WIDGET_HEIGHT     65
#define WIDGET_SEPARATOR  10
#define WIDGETS_PER_PAGE  6.0F

class Widget;
typedef struct { std::string title; Widget *widget; } WidgetPair;
typedef std::vector<WidgetPair> WidgetList;

class Widget {
public:
  static u16 g_selectedWidgetIndex;

  Widget(u8 addressSize, u8 valueSize);
  virtual ~Widget();

  static void drawWidgets(Gui *gui, WidgetList &widgets, u16 y, u16 start, u16 end);
  static void handleInput(u32 kdown, WidgetList &widgets);

  virtual void draw(Gui *gui, u16 x, u16 y) = 0;
  virtual void onInput(u32 kdown) = 0;
  virtual void onTouch(touchPosition &touch) = 0;

  u64 getValue();

  void setValue(u64 value);

  void setOffset(u32 offsetAddress, u32 address);
  void setOffset(u32 address);

private:
  u32 m_offsetAddress;
  u32 m_address;

protected:
  u8 m_addressSize, m_valueSize;
};
