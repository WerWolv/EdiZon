#include "gui_main.hpp"
#include "gui_editor.hpp"

#include "save.hpp"
#include "title.hpp"
#include "account.hpp"
#include <string>
#include <sstream>
#include <math.h>

extern std::unordered_map<u64, Title*> titles;
extern std::unordered_map<u128, Account*> accounts;

signed int xOffset;
float menuTimer = 0.0F;

enum {
  TITLE_SELECT,
  ACCOUNT_SELECT,
  FILE_SELECT
} selectionState;

GuiMain::GuiMain() : Gui() {
  m_selectedTitle = 0;
  xOffset = 0;
}

GuiMain::~GuiMain() {

}

void GuiMain::draw() {
  xOffset = m_selectedTitle > 5 ? m_selectedTitle > titles.size() - 5 ? 256 * ceil((titles.size() - (titles.size() > 10 ? 11.0F : 9.0F)) / 2) : 256 * ceil((m_selectedTitle - 5.0F) / 2) : 0;
  Gui::drawRectangle(0, 0, Gui::m_framebuffer_width, Gui::m_framebuffer_height, currTheme.backgroundColor);
  Gui::drawRectangle(0, 0, Gui::m_framebuffer_width, 10, COLOR_BLACK);

  signed int x = 0, y = 10, currItem = 0;
  u16 selectedX = 0, selectedY = 0;

  for(auto title : titles) {
    Gui::drawImage(x - xOffset, y, 256, 256, title.second->getTitleIcon(), IMAGE_MODE_RGB24);
    Gui::drawShadow(x - xOffset, y, 256, 256);

    if(currItem == m_selectedTitle) {
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

  u16 accountX = 0;

  for(auto account : accounts) {
    Gui::drawImage(50 + accountX, 500, 128, 128, account.second->getProfileImage(128), IMAGE_MODE_RGB24);
    accountX += 150;
  }


  menuTimer += 0.025;

  gfxFlushBuffers();
  gfxSwapBuffers();
  gfxWaitForVsync();
}

void GuiMain::onInput(u32 kdown) {
  menuTimer = 1.0F;

  if(kdown & KEY_LEFT) {
    if(((signed int)this->m_selectedTitle - 2) >= 0)
      this->m_selectedTitle -= 2;
  }

  if(kdown & KEY_RIGHT) {
    if((u16)(this->m_selectedTitle + 2) <= (titles.size() - 1))
      this->m_selectedTitle += 2;
  }

  if(kdown & KEY_UP || kdown & KEY_DOWN) {
    if((this->m_selectedTitle % 2) == 0) {
      if((u16)(this->m_selectedTitle + 1) <= (titles.size() - 1))
        this->m_selectedTitle++;
    }
    else this->m_selectedTitle--;
  }

  if(kdown & KEY_A) {
    Gui::g_currTitle = titles[m_selectedTitleId];
  }

}
