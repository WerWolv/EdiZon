#include "gui_main.hpp"
#include "save.hpp"
#include "title.hpp"
#include <unordered_map>
#include <string>

extern std::unordered_map<u64, Title*> titles;

GuiMain::GuiMain() : Gui() {
  this->m_selectedItem = 3;
}

GuiMain::~GuiMain() {

}

void GuiMain::draw() {
  Gui::drawRectangle(0, 0, Gui::m_framebuffer_width, Gui::m_framebuffer_height, currTheme.backgroundColor);

  //Title *t = titles.at(0x0100000000010000);
  Gui::drawImage(0, 0, 256, 256, titles.at(0x0100000000010000)->icon(), IMAGE_MODE_RGB24);
  /*else
  {
    char buf[30];
    sprintf(buf, "Failed to draw icon. Error: %d", (unsigned int)t->errorCode());
    Gui::drawText(font20, 0, 0, currTheme.textColor, buf);
  }*/

  //Gui::drawShadow(100, 50, 256, 256);
  gfxFlushBuffers();
  gfxSwapBuffers();
  gfxWaitForVsync();
}
