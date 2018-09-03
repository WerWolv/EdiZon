#include "gui_main.hpp"
#include "gui_editor.hpp"

#include "save.hpp"
#include "title.hpp"
#include "config_parser.hpp"
#include "account.hpp"

#include "threads.hpp"

#include <string>
#include <sstream>
#include <math.h>

int64_t xOffset, xOffsetNext;
bool finishedDrawing = true;

bool editableOnly = false;
int64_t editableCount = 0;

enum {
  TITLE_SELECT,
  ACCOUNT_SELECT
} selectionState = TITLE_SELECT;

GuiMain::GuiMain() : Gui() {
  m_selected.accountIndex = 0;
  selectionState = TITLE_SELECT;

  if (Title::g_titles.size() != 0)
    xOffset = m_selected.titleIndex > 5 ? m_selected.titleIndex > Title::g_titles.size() - 5 ? 256 * (ceil((Title::g_titles.size() - (Title::g_titles.size() >= 10 ? 11.0F : 9.0F)) / 2.0F) + (Title::g_titles.size() > 10 && Title::g_titles.size() % 2 == 1 ? 1 : 0)) : 256 * ceil((m_selected.titleIndex - 5.0F) / 2.0F) : 0;

  printf("Size: %lu", Title::g_titles.size());
  for (auto title : Title::g_titles) {
    if (ConfigParser::hasConfig(title.first)) {
      ConfigParser::g_editableTitles.insert({title.first, true});
    }
  }
}

GuiMain::~GuiMain() {

}

void GuiMain::update() {
  Gui::update();

  double deltaOffset = xOffsetNext - xOffset;
  double scrollSpeed = deltaOffset / 80.0F;

  if (xOffset != xOffsetNext && finishedDrawing) {
    if (xOffsetNext > xOffset)
      xOffset += ceil((abs(deltaOffset) > scrollSpeed) ? scrollSpeed : deltaOffset);
    else
      xOffset += floor((abs(deltaOffset) > scrollSpeed) ? scrollSpeed : deltaOffset);
  }
}

void GuiMain::draw() {
  Gui::beginDraw();

  Gui::drawRectangle(0, 0, Gui::g_framebuffer_width, Gui::g_framebuffer_height, currTheme.backgroundColor);
  Gui::drawRectangle(0, 0, Gui::g_framebuffer_width, 10, COLOR_BLACK);

  if (Title::g_titles.size() == 0) {
    Gui::drawTextAligned(font24, (Gui::g_framebuffer_width / 2), (Gui::g_framebuffer_height / 2), currTheme.textColor, "No games or saves found on this system! Please press \uE0EF to exit EdiZon!", ALIGNED_CENTER);
    Gui::endDraw();
    return;
  }

  int64_t x = 0, y = 10, currItem = 0;
  int64_t selectedX = 0, selectedY = 0;

  xOffsetNext = m_selected.titleIndex > 5 ? m_selected.titleIndex > Title::g_titles.size() - 5 ? 256 * (ceil((Title::g_titles.size() - (Title::g_titles.size() >= 10 ? 11.0F : 9.0F)) / 2.0F) + (Title::g_titles.size() > 10 && Title::g_titles.size() % 2 == 1 ? 1 : 0)) : 256 * ceil((m_selected.titleIndex - 5.0F) / 2.0F) : 0;

  finishedDrawing = false;
  editableCount = 0;

  for (auto title : Title::g_titles) {
    if (!editableOnly || ConfigParser::g_editableTitles.count(title.first)) {
      if (x - xOffset >= -256 && x - xOffset < Gui::g_framebuffer_width) {
        Gui::drawImage(x - xOffset, y, 256, 256, title.second->getTitleIcon(), IMAGE_MODE_RGB24);
        Gui::drawShadow(x - xOffset, y, 256, 256);
      }

      if (currItem == m_selected.titleIndex) {
        selectedX = x - xOffset;
        selectedY = y;
        m_selected.titleId = title.first;
      }

      y = y == 10 ? 266 : 10;

      currItem++;

      x = floor(currItem / 2.0F) * 256;

      editableCount++;
    }
  }

  finishedDrawing = true;

  if (editableOnly && editableCount == 0) {
    Gui::drawTextAligned(font24, (Gui::g_framebuffer_width / 2), (Gui::g_framebuffer_height / 2), currTheme.textColor, "No editable games found on this system!", ALIGNED_CENTER);
    Gui::endDraw();
    return;
  }
  else {
    if (selectionState >= TITLE_SELECT) {
      Gui::drawRectangled(selectedX - 10, selectedY - 10, 276, 276, selectionState == TITLE_SELECT ? currTheme.highlightColor : currTheme.selectedColor);
      Gui::drawRectangled(selectedX - 5, selectedY - 5, 266, 266, currTheme.selectedButtonColor);
      Gui::drawImage(selectedX, selectedY, 256, 256, Title::g_titles[m_selected.titleId]->getTitleIcon(), IMAGE_MODE_RGB24);
      Gui::drawShadow(selectedX - 10, selectedY - 10, 276, 276);
    }
  }

  if (selectionState == TITLE_SELECT) {
    Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 2, 605, currTheme.textColor, "Select title and account by pressing \uE0E0 or update all config and script files by pressing \uE0F0", ALIGNED_CENTER);
    if (editableOnly)
      Gui::drawText(font20, 20, 675, currTheme.textColor, "\uE0E4 Showing editable titles");
    else
      Gui::drawText(font20, 20, 675, currTheme.textColor, "\uE0E4 Showing all titles");
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

  Gui::endDraw();
}

void GuiMain::onInput(u32 kdown) {
  if (Title::g_titles.size() == 0) return;

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

  if (kdown & KEY_L) {
    editableOnly = !editableOnly;
    Gui::g_nextGui = GUI_MAIN;
  }

  if (kdown & KEY_B) {
    if (selectionState == ACCOUNT_SELECT) {
        selectionState = TITLE_SELECT;
        m_selected.accountIndex = 0;
    }
  }
  if (kdown & KEY_MINUS) {
    (new MessageBox("Checking for updates...", MessageBox::NONE))->show();
    GuiMain::g_shouldUpdate = true;
  }
}

void GuiMain::onTouch(touchPosition &touch) {
  if (Title::g_titles.size() == 0) return;

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

void GuiMain::onGesture(touchPosition &startPosition, touchPosition &endPosition) {
  if (Title::g_titles.size() == 0) return;

}
