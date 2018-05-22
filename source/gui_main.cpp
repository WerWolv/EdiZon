#include "gui_main.hpp"
#include "gui_editor.hpp"

#include "save.hpp"
#include "title.hpp"
#include <unordered_map>
#include <string>
#include <sstream>
#include <math.h>

extern std::unordered_map<u64, Title*> titles;
signed int xOffset;
float menuTimer = 0.0F;

GuiMain::GuiMain() : Gui() {
  m_selectedItem = 0;
  xOffset = 0;
}

GuiMain::~GuiMain() {

}

void GuiMain::draw() {
  xOffset = m_selectedItem > 5 ? m_selectedItem > titles.size() - 5 ? 256 * ceil((titles.size() - (titles.size() > 10 ? 11.0F : 9.0F)) / 2) : 256 * ceil((m_selectedItem - 5.0F) / 2) : 0;
  Gui::drawRectangle(0, 0, Gui::m_framebuffer_width, Gui::m_framebuffer_height, currTheme.backgroundColor);
  Gui::drawRectangle(0, 0, Gui::m_framebuffer_width, 10, COLOR_BLACK);

  signed int x = 0, y = 10, currItem = 0;
  u16 selectedX = 0, selectedY = 0;

  for(auto title : titles) {
    Gui::drawImage(x - xOffset, y, 256, 256, title.second->getTitleIcon(), IMAGE_MODE_RGB24);
    Gui::drawShadow(x - xOffset, y, 256, 256);

    if(currItem == m_selectedItem) {
      selectedX = x - xOffset;
      selectedY = y;
      m_selectedTitleId = title.first;
    }

    y = y == 10 ? 266 : 10;
    if(y == 10)
      x += 256;

    currItem++;
  }

  float highlightMultiplier = fmax(0.5, fabs(fmod(menuTimer, 1.0) - 0.5) / 0.5);
  color_t highlightColor = currTheme.highlightColor;
  highlightColor.a = 0xE0 * highlightMultiplier;
  Gui::drawRectangled(selectedX - 10, selectedY - 10, 276, 276, highlightColor);
  Gui::drawRectangled(selectedX - 5, selectedY - 5, 266, 266, currTheme.spacerColor);
  Gui::drawImage(selectedX, selectedY, 256, 256, titles[m_selectedTitleId]->getTitleIcon(), IMAGE_MODE_RGB24);
  Gui::drawShadow(selectedX, selectedY, 256, 256);


  menuTimer += 0.025;

  gfxFlushBuffers();
  gfxSwapBuffers();
  gfxWaitForVsync();
}

void GuiMain::onInput(u32 kdown) {
  menuTimer = 1.0F;

  if(kdown & KEY_LEFT) {
    if(((signed int)this->m_selectedItem - 2) >= 0)
      this->m_selectedItem -= 2;
  }

  if(kdown & KEY_RIGHT) {
    if((u16)(this->m_selectedItem + 2) <= (titles.size() - 1))
      this->m_selectedItem += 2;
  }

  if(kdown & KEY_UP || kdown & KEY_DOWN) {
    if((this->m_selectedItem % 2) == 0) {
      if((u16)(this->m_selectedItem + 1) <= (titles.size() - 1))
        this->m_selectedItem++;
    }
    else this->m_selectedItem--;
  }

  if(kdown & KEY_A) {
    Gui::g_currTitle = titles[m_selectedTitleId];
    Gui::g_nextGui = GUI_EDITOR;
  }

}
