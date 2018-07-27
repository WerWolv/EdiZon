#pragma once

#include <string>
#include <functional>

#include <switch.h>

class Gui;

class Keyboard {
public:
  Keyboard(std::string title, u8 maxInputLength);
  ~Keyboard();

  void show();
  void hide();

  void update();
  void draw(Gui *gui);
  void onInput(u32 kdown);

  Keyboard* setInputFinishedAction(std::function<void(std::string)> inputFinishedAction);

private:
  std::string m_title;
  u8 m_maxInputLength;
  u8 m_cursorPosition = 0;

  std::string m_enteredString;

  bool m_capsEnabled = false;
  bool m_capsLockEnabled = false;

  std::function<void(std::string)> m_inputFinishedAction;
};
