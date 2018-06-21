#pragma once

#include <switch.h>

#include <string>
#include <utility>

#include "gui.hpp"

#define WIDGET_WIDTH      300
#define WIDGET_HEIGHT     65
#define WIDGET_SEPARATOR  10

#define CONFIG_ROOT "/EdiZon/editor/"

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
  static bool getList(WidgetList& list, std::vector<std::tuple<std::string, size_t, u8*>> files);

  virtual void draw(Gui *gui, u16 x, u16 y) = 0;
  virtual void onInput(u32 kdown) = 0;
  virtual void onTouch(touchPosition &touch) = 0;

  u16 getValue();
  void setValue(u16 value);

protected:
  u16 m_value, m_address;
  u8** m_buffer;
};
