#include "gui_main.hpp"

GuiMain::GuiMain() : Gui() {

}

GuiMain::~GuiMain() {
  
}

void GuiMain::draw() {
  Gui::drawRectangle(0, 0, Gui::m_framebuffer_width, Gui::m_framebuffer_height, currTheme.backgroundColor);

  Gui::drawText(font20, 100, 100, currTheme.textColor, "HELLO WORLD!");


  gfxFlushBuffers();
  gfxSwapBuffers();
  gfxWaitForVsync();
}
