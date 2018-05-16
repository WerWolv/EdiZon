#include "gui_main.hpp"
#include "save.hpp"

GuiMain::GuiMain() : Gui() {
}

GuiMain::~GuiMain() {

}

void GuiMain::draw() {
  Gui::drawRectangle(0, 0, Gui::m_framebuffer_width, Gui::m_framebuffer_height, currTheme.backgroundColor);

  uint8_t* ptr;
  bool ret = getTitleIcon(0x0100000000010000, &ptr); // SMO

  if (ret)
  {
    Gui::drawImage(0, 0, 256, 256, ptr, IMAGE_MODE_RGB24);
  }
  else
    Gui::drawText(font20, 0, 0, currTheme.textColor, "Failed to get the title icon.");

  gfxFlushBuffers();
  gfxSwapBuffers();
  gfxWaitForVsync();
}
