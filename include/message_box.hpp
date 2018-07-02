#pragma once

#include <switch.h>
#include <string>

class Gui;

typedef enum {
  OKAY,
  YES_NO
} MessageBoxOptions;

typedef enum {
  INFORMATION,
  WARNING,
  ERROR
} MessageBoxStyle;

class MessageBox {
public:
  MessageBox(std::string title, std::string message, MessageBoxOptions options, MessageBoxStyle style);
  ~MessageBox();

  void draw(Gui *gui);

  void onInput(u32 kdown);
  void onTouch(touchPosition &touch);

  void show();
  void hide();

private:
  std::string m_title, m_message;
  MessageBoxOptions m_options;
  MessageBoxStyle m_style;

  u8 m_selectedOption;
};
