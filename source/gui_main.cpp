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
} selectionState = TITLE_SELECT;

GuiMain::GuiMain() : Gui() {
  m_selected.titleIndex = 0;
  m_selected.accountIndex = 0;
  xOffset = 0;
}

GuiMain::~GuiMain() {

}

void GuiMain::draw() {
  xOffset = m_selected.titleIndex > 5 ? m_selected.titleIndex > titles.size() - 5 ? 256 * ceil((titles.size() - (titles.size() > 10 ? 11.0F : 9.0F)) / 2) : 256 * ceil((m_selected.titleIndex - 5.0F) / 2) : 0;
  Gui::drawRectangle(0, 0, Gui::m_framebuffer_width, Gui::m_framebuffer_height, currTheme.backgroundColor);
  Gui::drawRectangle(0, 0, Gui::m_framebuffer_width, 10, COLOR_BLACK);

  signed int x = 0, y = 10, currItem = 0;
  u16 selectedX = 0, selectedY = 0;

  for(auto title : titles) {
    Gui::drawImage(x - xOffset, y, 256, 256, title.second->getTitleIcon(), IMAGE_MODE_RGB24);
    Gui::drawShadow(x - xOffset, y, 256, 256);

    if(currItem == m_selected.titleIndex) {
      selectedX = x - xOffset;
      selectedY = y;
      m_selected.titleId = title.first;
    }

    y = y == 10 ? 266 : 10;
    if(y == 10)
      x += 256;

    currItem++;
  }

  float highlightMultiplier = fmax(0.5, fabs(fmod(menuTimer, 1.0) - 0.5) / 0.5);
  color_t highlightColor = currTheme.highlightColor;
  highlightColor.a = 0xE0 * highlightMultiplier;

  switch(selectionState) {
    case TITLE_SELECT:
      Gui::drawRectangled(selectedX - 10, selectedY - 10, 276, 276, highlightColor);
      Gui::drawRectangled(selectedX - 5, selectedY - 5, 266, 266, currTheme.spacerColor);
      Gui::drawImage(selectedX, selectedY, 256, 256, titles[m_selected.titleId]->getTitleIcon(), IMAGE_MODE_RGB24);
      Gui::drawShadow(selectedX, selectedY, 256, 256);
      break;
    case ACCOUNT_SELECT:
      Gui::drawRectangled(40 + m_selected.accountIndex * 150 - 10, 550, 148, 148, highlightColor);
      Gui::drawRectangled(40 + m_selected.accountIndex * 150 - 5, 555, 138, 138, currTheme.spacerColor);
      Gui::drawImage(40 + m_selected.accountIndex * 150, 560, 128, 128, titles[m_selected.titleId]->getTitleIcon(), IMAGE_MODE_RGB24);
      Gui::drawShadow(40 + m_selected.accountIndex * 150, 560, 128, 128);
      break;
  }

  u16 accountX = 0;

  for(u128 userID : titles[m_selected.titleId]->getUserIDs()) {
    Gui::drawImage(40 + accountX, 560, 128, 128, accounts[userID]->getProfileImage(), IMAGE_MODE_RGB24);
    Gui::drawShadow(40 + accountX, 560, 128, 128);
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
    if(selectionState == TITLE_SELECT) {
      if(((signed int)m_selected.titleIndex - 2) >= 0)
        m_selected.titleIndex -= 2;
    } else if(selectionState == ACCOUNT_SELECT) {
      if(((signed int)m_selected.accountIndex - 1) >= 0)
        m_selected.accountIndex--;
    }
  }

  if(kdown & KEY_RIGHT) {
    if(selectionState == TITLE_SELECT) {
      if((u16)(m_selected.titleIndex + 2) <= (titles.size() - 1))
        m_selected.titleIndex += 2;
    } else if(selectionState == ACCOUNT_SELECT) {
      if((u16)(m_selected.accountIndex + 1) <= (titles[m_selected.titleId]->getUserIDs().size() - 1))
        m_selected.accountIndex++;
    }
  }

  if(kdown & KEY_UP || kdown & KEY_DOWN) {
    if(selectionState == TITLE_SELECT) {
      if((m_selected.titleIndex % 2) == 0) {
        if((u16)(m_selected.titleIndex + 1) <= (titles.size() - 1))
          m_selected.titleIndex++;
      }
      else m_selected.titleIndex--;
    }
  }

  if(kdown & KEY_A) {
    Gui::g_currTitle = titles[m_selected.titleId];

    if(selectionState == TITLE_SELECT)
      selectionState = ACCOUNT_SELECT;
    else if(selectionState == ACCOUNT_SELECT)
      selectionState = FILE_SELECT;
    else if(selectionState == FILE_SELECT)
      Gui::g_nextGui = GUI_EDITOR;
  }

  if(kdown & KEY_B) {
    if(selectionState == ACCOUNT_SELECT) {
        selectionState = TITLE_SELECT;
        m_selected.accountIndex = 0;
    }
    else if(selectionState == FILE_SELECT)
      selectionState = ACCOUNT_SELECT;
  }

}
