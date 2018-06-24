#include "gui_main.hpp"
#include "gui_editor.hpp"

#include "save.hpp"
#include "title.hpp"
#include "account.hpp"
#include <string>
#include <sstream>
#include <math.h>

float xOffset;
float xOffsetNext;

enum {
  TITLE_SELECT,
  ACCOUNT_SELECT
} selectionState = TITLE_SELECT;

GuiMain::GuiMain() : Gui() {
  m_selected.titleIndex = 0;
  m_selected.accountIndex = 0;
  selectionState = TITLE_SELECT;
  xOffset = 0;
}

GuiMain::~GuiMain() {

}

void GuiMain::draw() {
  float deltaOffset = xOffsetNext - xOffset;
  float scrollSpeed = deltaOffset / 4.0F;

  Gui::beginDraw();

  xOffsetNext = m_selected.titleIndex > 5 ? m_selected.titleIndex > Title::g_titles.size() - 5 ? 256 * (ceil((Title::g_titles.size() - (Title::g_titles.size() >= 10 ? 11.0F : 9.0F)) / 2.0F) + (Title::g_titles.size() > 10 && Title::g_titles.size() % 2 == 1 ? 1 : 0)) : 256 * ceil((m_selected.titleIndex - 5.0F) / 2.0F) : 0;
  Gui::drawRectangle(0, 0, Gui::framebuffer_width, Gui::framebuffer_height, currTheme.backgroundColor);
  Gui::drawRectangle(0, 0, Gui::framebuffer_width, 10, COLOR_BLACK);

  float x = 0, y = 10, currItem = 0;
  float selectedX = 0, selectedY = 0;

  for (auto title : Title::g_titles) {
    Gui::drawImage(x - xOffset, y, 256, 256, title.second->getTitleIcon(), IMAGE_MODE_RGB24);
    Gui::drawShadow(x - xOffset, y, 256, 256);

    if (currItem == m_selected.titleIndex) {
      selectedX = x - xOffset;
      selectedY = y;
      m_selected.titleId = title.first;
    }

    y = y == 10 ? 266 : 10;

    if (y == 10)
      x += 256;

    currItem++;
  }

  if (selectionState >= TITLE_SELECT) {
    Gui::drawRectangled(selectedX - 10, selectedY - 10, 276, 276, selectionState == TITLE_SELECT ? currTheme.highlightColor : currTheme.selectedColor);
    Gui::drawRectangled(selectedX - 5, selectedY - 5, 266, 266, currTheme.selectedButtonColor);
    Gui::drawImage(selectedX, selectedY, 256, 256, Title::g_titles[m_selected.titleId]->getTitleIcon(), IMAGE_MODE_RGB24);
    Gui::drawShadow(selectedX - 10, selectedY - 10, 276, 276);
  }

  if (selectionState >= ACCOUNT_SELECT && Title::g_titles[m_selected.titleId]->getUserIDs().size() > 0) {

      for (u8 i = 0; i < Title::g_titles[m_selected.titleId]->getUserIDs().size(); i++)
        Gui::drawShadow(40 + i * 150, 560, 128, 128);

      Gui::drawRectangled(40 + m_selected.accountIndex * 150 - 10, 550, 148, 148, currTheme.highlightColor);
      Gui::drawRectangled(40 + m_selected.accountIndex * 150 - 5, 555, 138, 138, currTheme.selectedButtonColor);
      Gui::drawShadow(40 + m_selected.accountIndex * 150 - 10, 550, 148, 148);

      u16 accountX = 0;

      for (u128 userID : Title::g_titles[m_selected.titleId]->getUserIDs()) {
        Gui::drawImage(40 + accountX, 560, 128, 128, Account::g_accounts[userID]->getProfileImage(), IMAGE_MODE_RGB24);
        accountX += 150;
      }
  }

  if (xOffset != xOffsetNext) {
    if (xOffsetNext > xOffset)
      xOffset += ceil((abs(deltaOffset) > scrollSpeed) ? scrollSpeed : deltaOffset);
    else
      xOffset += floor((abs(deltaOffset) > scrollSpeed) ? scrollSpeed : deltaOffset);
  }


  Gui::endDraw();
}

void GuiMain::onInput(u32 kdown) {
  if (kdown & KEY_LEFT) {
    if (selectionState == TITLE_SELECT) {
      if (static_cast<s16>(m_selected.titleIndex - 2) >= 0)
        m_selected.titleIndex -= 2;
    } else if (selectionState == ACCOUNT_SELECT) {
      if (static_cast<s16>(m_selected.accountIndex - 1) >= 0)
        m_selected.accountIndex--;
    }
  }

  if (kdown & KEY_RIGHT) {
    if (selectionState == TITLE_SELECT) {
      if (static_cast<u16>(m_selected.titleIndex + 2) < Title::g_titles.size())
        m_selected.titleIndex += 2;
    } else if (selectionState == ACCOUNT_SELECT) {
      if (static_cast<u16>(m_selected.accountIndex + 1) < Title::g_titles[m_selected.titleId]->getUserIDs().size())
        m_selected.accountIndex++;
    }
  }

  if (kdown & KEY_UP || kdown & KEY_DOWN) {
    if (selectionState == TITLE_SELECT) {
      if ((m_selected.titleIndex % 2) == 0) {
        if (static_cast<u16>(m_selected.titleIndex + 1) < Title::g_titles.size())
          m_selected.titleIndex++;
      }
      else m_selected.titleIndex--;
    }
  }

  if (kdown & KEY_A) {
    if (selectionState == TITLE_SELECT)
      selectionState = ACCOUNT_SELECT;
    else if (selectionState == ACCOUNT_SELECT && Title::g_titles[m_selected.titleId]->getUserIDs().size() > 0) {
      Title::g_currTitle = Title::g_titles[m_selected.titleId];
      Account::g_currAccount = Account::g_accounts[Title::g_titles[m_selected.titleId]->getUserIDs()[m_selected.accountIndex]];
      Gui::g_nextGui = GUI_EDITOR;
    }
  }

  if (kdown & KEY_B) {
    if (selectionState == ACCOUNT_SELECT) {
        selectionState = TITLE_SELECT;
        m_selected.accountIndex = 0;
    }
  }
}

void GuiMain::onTouch(touchPosition &touch) {
  switch (selectionState) {
    case TITLE_SELECT: {
      u8 x = floor((touch.px + xOffset) / 256.0F);
      u8 y = floor(touch.py / 256.0F);
      u8 title = y + x * 2;

      if (y <= 1 && title < Title::g_titles.size()) {
        if (m_selected.titleIndex == title) {
          Title::g_currTitle = Title::g_titles[m_selected.titleId];
          selectionState = ACCOUNT_SELECT;
        }
        m_selected.titleIndex = title;
      }
      break;
    }
    case ACCOUNT_SELECT: {
      u8 account = floor((touch.px - 40) / 150.0F);

      if (account < Title::g_titles[m_selected.titleId]->getUserIDs().size() && touch.py > 560 && touch.py < (560 + 128)) {
        if (m_selected.accountIndex == account) {
          m_selected.userId = Title::g_titles[m_selected.titleId]->getUserIDs()[account];
          Title::g_currTitle = Title::g_titles[m_selected.titleId];
          Account::g_currAccount = Account::g_accounts[Title::g_titles[m_selected.titleId]->getUserIDs()[m_selected.accountIndex]];
          Gui::g_nextGui = GUI_EDITOR;
        }
        m_selected.accountIndex = account;
      } else {
        selectionState = TITLE_SELECT;
        m_selected.accountIndex = 0;
      }

      break;
    }
  }
}
