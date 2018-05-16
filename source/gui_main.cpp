#include "gui_main.hpp"
#include "save.hpp"
#include "title.hpp"
#include <unordered_map>

extern std::unordered_map<uint64_t, void*> titles;

GuiMain::GuiMain() : Gui() {
  this->m_selectedItem = 3;
}

GuiMain::~GuiMain() {

}

void GuiMain::draw() {
  Gui::drawRectangle(0, 0, Gui::m_framebuffer_width, Gui::m_framebuffer_height, currTheme.backgroundColor);

  Gui::drawImage(100, 100, 256, 256, ((Title*)titles.at(0x0100000000010000))->getTitleIcon(), IMAGE_MODE_RGB24);
  Gui::drawShadow(100, 50, 256, 256);
  gfxFlushBuffers();
  gfxSwapBuffers();
  gfxWaitForVsync();
}
