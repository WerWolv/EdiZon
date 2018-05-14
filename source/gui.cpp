#include "gui.hpp"

Gui::Gui() {
  this->m_framebuffer = (uint32_t*) gfxGetFramebuffer(&this->m_framebuffer_width, &this->m_framebuffer_height);
}

Gui::~Gui() {

}

void Gui::draw() {
  for(uint32_t i = 0; i < this->m_framebuffer_width; i++) {
    for(uint32_t j = 0; j < this->m_framebuffer_height; j++) {
      this->m_framebuffer[i + j * this->m_framebuffer_width] = RGBA8(0xAA, 0xAA, 0xAA, 0xFF);
    }
  }

  gfxFlushBuffers();
  gfxSwapBuffers();
  gfxWaitForVsync();
}
