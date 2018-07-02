#include "snackbar.hpp"

#include "gui.hpp"

#define SNACKBAR_HEIGHT 100
#define DISPLAY_TIME    200

u16 yOffset = 0;

Snackbar::Snackbar(std::string text) : m_text(text) {
  m_isDead = false;
}

Snackbar::~Snackbar() {

}

void Snackbar::show() {
  m_displayTime = DISPLAY_TIME;
  Gui::g_currSnackbar = this;
}

void Snackbar::draw(Gui *gui) {
  if (--m_displayTime == 0)
    m_isDead = true;

  if (m_displayTime < 20)
    yOffset = SNACKBAR_HEIGHT - m_displayTime * 5;
  else if (m_displayTime > (DISPLAY_TIME - 20))
    yOffset = SNACKBAR_HEIGHT - (DISPLAY_TIME - m_displayTime) * 5;
  else yOffset = 0;

  gui->drawRectangle(0, (Gui::g_framebuffer_height - SNACKBAR_HEIGHT) + yOffset, Gui::g_framebuffer_width, Gui::g_framebuffer_height, currTheme.textColor);
  gui->drawText(font20, 50, (Gui::g_framebuffer_height - SNACKBAR_HEIGHT) + 35 + yOffset, currTheme.backgroundColor, m_text.c_str());
}

bool Snackbar::isDead() {
  return m_isDead;
}
