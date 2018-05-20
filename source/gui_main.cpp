#include "gui_main.hpp"
#include "gui_editor.hpp"

#include "save.hpp"
#include "title.hpp"
#include <unordered_map>
#include <string>

extern std::unordered_map<u64, Title*> titles;

GuiMain::GuiMain() : Gui() {
  this->m_selectedItem = 0;
}

GuiMain::~GuiMain() {

}

void GuiMain::draw() {
  Gui::drawRectangle(0, 0, Gui::m_framebuffer_width, Gui::m_framebuffer_height, currTheme.backgroundColor);

  u16 x = 0, y = 0, currItem = 0;
  u16 yOffset = this->m_selectedItem >= 10 ? (256 * 3) - this->m_framebuffer_height + ((this->m_selectedItem - 14) / 5) * 256 : 0;

  for(auto title : titles) {
    Gui::drawImage(x, y - yOffset, 256, 256, title.second->getTitleIcon(), IMAGE_MODE_RGB24);

    if(currItem == this->m_selectedItem) {
      Gui::drawRectangled(x, y - yOffset, 256, 256, Gui::makeColor(0x00, 0x40, 0xFF, 0x80));
      this->m_selectedTitleId = title.first;
    }

    x += 256;
    if(x >= m_framebuffer_width) {
      x = 0;
      y += 256;
    }
    currItem++;
  }

  gfxFlushBuffers();
  gfxSwapBuffers();
  gfxWaitForVsync();
}

void GuiMain::onInput(u32 kdown) {
  if(kdown & KEY_LEFT) {
    if(this->m_selectedItem == 0)
      this->m_selectedItem = titles.size() - 1;
      else this->m_selectedItem--;
  }

  if(kdown & KEY_RIGHT) {
    if(this->m_selectedItem == (titles.size() - 1))
      this->m_selectedItem = 0;
      else this->m_selectedItem++;
  }

  if(kdown & KEY_UP) {
    if(this->m_selectedItem > 4)
      this->m_selectedItem -= 5;
  }

  if(kdown & KEY_DOWN) {
    if(this->m_selectedItem < (titles.size() - 5))
      this->m_selectedItem += 5;
  }

  if(kdown & KEY_A) {
    Gui::g_currTitle = titles[m_selectedTitleId];
    Gui::g_nextGui = GUI_EDITOR;
  }

}
