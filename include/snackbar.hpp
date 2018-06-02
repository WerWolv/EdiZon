#pragma once

#include <switch.h>
#include <string>

class Gui;

class Snackbar {
public:
  Snackbar(Gui *gui, std::string text);
  ~Snackbar();

  void show();
  void draw();

  bool isDead();

private:
  std::string m_text;
  Gui *m_gui;
  u16 m_displayTime;
  bool m_isDead;
};
