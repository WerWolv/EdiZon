#pragma once

#include <switch.h>
#include <string>
#include <functional>

class Gui;

class MessageBox {
public:
  typedef enum {
    NONE = 0,
    OKAY = 1,
    YES_NO = 2
  } MessageBoxOptions;

  MessageBox(std::string message, MessageBox::MessageBoxOptions options);
  ~MessageBox();

  MessageBox* setSelectionAction(std::function<void(s8)> selectionAction);
  MessageBox* setCustomDraw(std::function<void(Gui *gui, s16 x, s16 y)> customDraw);

  void draw(Gui *gui);
  void onInput(u32 kdown);
  void onTouch(touchPosition &touch);
  void onGesture(touchPosition startPosition, touchPosition endPosition, bool finish);
  
  void setProgress(s8 progress);

  void show();
  void hide();

private:
  std::string m_message;
  MessageBoxOptions m_options;

  u8 m_selectedOption;
  s8 m_progress;
  std::function<void(s8)> m_selectionAction;
  std::function<void(Gui *gui, s16 x, s16 y)> m_customDraw;
};
