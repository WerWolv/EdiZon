#pragma once

#include <switch.h>
#include <string>
#include <functional>

class Gui;

typedef enum {
  OKAY = 1,
  YES_NO = 2
} MessageBoxOptions;

class MessageBox {
public:
  MessageBox(std::string message, MessageBoxOptions options);
  ~MessageBox();

  MessageBox* setSelectionAction(std::function<void(s8)> selectionAction);

  void draw(Gui *gui);
  void onInput(u32 kdown);
  void onTouch(touchPosition &touch);

  void show();
  void hide();

private:
  std::string m_message;
  MessageBoxOptions m_options;

  u8 m_selectedOption;
  std::function<void(s8)> m_selectionAction;
};
