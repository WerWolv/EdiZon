#pragma once

#include <switch.h>
#include <string.h>

class Gui {
public:

  Gui();
  ~Gui();
  void draw();

private:
  uint32_t *m_framebuffer;
  uint32_t m_framebuffer_width;
  uint32_t m_framebuffer_height;
};
