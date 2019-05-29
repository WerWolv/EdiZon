#pragma once

#include "guis/gui.hpp"

#include <vector>
#include <unordered_map>
#include <stdbool.h>

#define GUIDE_PAGE_CNT 10

class GuiGuide : public Gui {
public:
  GuiGuide();
  ~GuiGuide();

  void update();
  void draw();
  void onInput(u32 kdown);
  void onTouch(touchPosition &touch);
  void onGesture(touchPosition startPosition, touchPosition endPosition, bool finish);

private:
  static inline u16 g_selectedPage = 1;

  typedef struct {
    char *data;
    std::string title;
    size_t length;
    u16 x, y;
    u16 w, h;
    enum { TYPE_IMAGE, TYPE_TEXT } type;
  } guide_obj_t;

  std::vector<guide_obj_t> m_objects;
};
