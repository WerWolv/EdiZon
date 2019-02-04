#pragma once

#include "guis/gui.hpp"

#include <vector>
#include <unordered_map>

#define MENU_SCROLL_SPEED 40

class GuiMain : public Gui {
public:
  GuiMain();
  ~GuiMain();

  static inline bool g_shouldUpdate = false;

  void update();
  void draw();
  void onInput(u32 kdown);
  void onTouch(touchPosition &touch);
  void onGesture(touchPosition startPosition, touchPosition endPosition, bool finish);

private:
  static inline struct {
    s16 titleIndex;

    u64 titleId;
    u128 userId;
  } m_selected;

  static inline bool m_editableOnly = false;
  bool m_backupAll = false;
  u16 m_editableCount = 0;
};
