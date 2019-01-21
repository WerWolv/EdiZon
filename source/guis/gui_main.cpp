#include "guis/gui_main.hpp"

#include "save.hpp"
#include "title.hpp"
#include "config_parser.hpp"
#include "account.hpp"

#include "beta_bin.h"

#include "threads.hpp"

#include <string>
#include <sstream>
#include <math.h>

int64_t xOffset, xOffsetNext;
bool finishedDrawing = true;

GuiMain::GuiMain() : Gui() {
  m_selected.accountIndex = 0;

  if (Title::g_titles.size() != 0)
    xOffset = m_selected.titleIndex > 5 ? m_selected.titleIndex > Title::g_titles.size() - 5 ? 256 * (ceil((Title::g_titles.size() - (Title::g_titles.size() >= 10 ? 11.0F : 9.0F)) / 2.0F) + (Title::g_titles.size() > 10 && Title::g_titles.size() % 2 == 1 ? 1 : 0)) : 256 * ceil((m_selected.titleIndex - 5.0F) / 2.0F) : 0;

  for (auto title : Title::g_titles) {
    if (ConfigParser::hasConfig(title.first) == 0) {
      ConfigParser::g_editableTitles.insert({title.first, true});
    }
  }
}

GuiMain::~GuiMain() {

}

void GuiMain::update() {
  Gui::update();

  if (xOffset != xOffsetNext && finishedDrawing) {
    double deltaOffset = xOffsetNext - xOffset;
    double scrollSpeed = deltaOffset / 40.0F;

    if (xOffsetNext > xOffset)
      xOffset += ceil((abs(deltaOffset) > scrollSpeed) ? scrollSpeed : deltaOffset);
    else
      xOffset += floor((abs(deltaOffset) > scrollSpeed) ? scrollSpeed : deltaOffset);
  }
}

void GuiMain::draw() {
  int64_t x = 0, y = 10, currItem = 0;
  int64_t selectedX = 0, selectedY = 0;
  bool tmpEditableOnly = m_editableOnly;

  Gui::beginDraw();

  finishedDrawing = false;

  Gui::drawRectangle(0, 0, Gui::g_framebuffer_width, Gui::g_framebuffer_height, currTheme.backgroundColor);
  Gui::drawRectangle(0, 0, Gui::g_framebuffer_width, 10, COLOR_BLACK);

  if (Title::g_titles.size() == 0) {
    Gui::drawTextAligned(font24, (Gui::g_framebuffer_width / 2), (Gui::g_framebuffer_height / 2), currTheme.textColor, "No games or saves found on this system! Please press \uE0EF to exit EdiZon!", ALIGNED_CENTER);
    Gui::endDraw();
    return;
  }

  xOffsetNext = m_selected.titleIndex > 5 ? m_selected.titleIndex > Title::g_titles.size() - 5 ? 256 * (ceil((Title::g_titles.size() - (Title::g_titles.size() >= 10 ? 11.0F : 9.0F)) / 2.0F) + (Title::g_titles.size() > 10 && Title::g_titles.size() % 2 == 1 ? 1 : 0)) : 256 * ceil((m_selected.titleIndex - 5.0F) / 2.0F) : 0;

  if (Title::g_titles.size() < 10) xOffsetNext = 0;
  if (m_editableCount < 10 && tmpEditableOnly) xOffsetNext = 0;

  m_editableCount = 0;

  for (auto title : Title::g_titles) {
    if (currItem == m_selected.titleIndex) {
      selectedX = x - xOffset;
      selectedY = y;
      m_selected.titleId = title.first;
    }

    if (!tmpEditableOnly || ConfigParser::g_editableTitles.count(title.first)) {
      if (x - xOffset >= -256 && x - xOffset < Gui::g_framebuffer_width) {
        Gui::drawImage(x - xOffset, y, 256, 256, title.second->getTitleIcon(), IMAGE_MODE_RGB24);

        if (ConfigParser::g_betaTitles[title.first])
          Gui::drawImage(x - xOffset, y, 150, 150, 256, 256, beta_bin, IMAGE_MODE_ABGR32);

        if (y == 266 || title.first == (--Title::g_titles.end())->first)
          Gui::drawShadow(x - xOffset, y, 256, 256);
      }

      y = y == 10 ? 266 : 10;
      x = floor(++currItem / 2.0F) * 256;

      m_editableCount++;
    }
  }

  if (tmpEditableOnly && m_editableCount == 0) {
    Gui::drawTextAligned(font24, (Gui::g_framebuffer_width / 2), (Gui::g_framebuffer_height / 2), currTheme.textColor, "No editable games found on this system!", ALIGNED_CENTER);
    Gui::endDraw();
    return;
  }
  else {
      Gui::drawRectangled(selectedX - 5, selectedY - 5, 266, 266, currTheme.highlightColor);
      Gui::drawImage(selectedX, selectedY, 256, 256, Title::g_titles[m_selected.titleId]->getTitleIcon(), IMAGE_MODE_RGB24);

      if (ConfigParser::g_betaTitles[m_selected.titleId])
        Gui::drawImage(selectedX, selectedY, 150, 150, 256, 256, beta_bin, IMAGE_MODE_ABGR32);

      Gui::drawShadow(selectedX - 5, selectedY - 5, 266, 266);

    Gui::drawRectangle((u32)((Gui::g_framebuffer_width - 1220) / 2), Gui::g_framebuffer_height - 73, 1220, 1, currTheme.textColor);

    if (tmpEditableOnly)
      Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 50, Gui::g_framebuffer_height - 50, currTheme.textColor, "\uE0E6 All titles     \uE0E7 + \uE0E2 Backup all     \uE0E2 Backup     \uE0EF Exit     \uE0F0 Update     \uE0E1 Back     \uE0E0 OK", ALIGNED_RIGHT);
    else
      Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 50, Gui::g_framebuffer_height - 50, currTheme.textColor, "\uE0E6 Editable titles     \uE0E7 + \uE0E2 Backup all     \uE0E2 Backup     \uE0EF Exit     \uE0F0 Update     \uE0E1 Back     \uE0E0 OK", ALIGNED_RIGHT);
  }

  finishedDrawing = true;

  Gui::endDraw();
}

void GuiMain::onInput(u32 kdown) {
  static bool batchClicked = false;

  if (Title::g_titles.size() == 0) return;

  if (kdown & KEY_LEFT) {
    if (static_cast<s16>(m_selected.titleIndex - 2) >= 0)
      m_selected.titleIndex -= 2;
  } else if (kdown & KEY_RIGHT) {
    if (static_cast<u16>(m_selected.titleIndex + 2) < ((!m_editableOnly) ?  Title::g_titles.size() : ConfigParser::g_editableTitles.size()))
      m_selected.titleIndex += 2;
  } else if (kdown & KEY_UP) {
    if ((m_selected.titleIndex % 2) == 1) {
          m_selected.titleIndex--;
    }
  } else if (kdown & KEY_DOWN) {
    if ((m_selected.titleIndex % 2) == 0) {
      if (static_cast<u16>(m_selected.titleIndex + 1) < ((!m_editableOnly) ?  Title::g_titles.size() : ConfigParser::g_editableTitles.size()))
        m_selected.titleIndex++;
    }
  }

  if (kdown & KEY_A) {
    u128 userID = Gui::requestPlayerSelection();
    std::vector<u128> users = Title::g_titles[m_selected.titleId]->getUserIDs();

    if(userID == 0x00)
      return;

    if (std::find(users.begin(), users.end(), userID) != users.end()) {
      Title::g_currTitle = Title::g_titles[m_selected.titleId];
      Account::g_currAccount = Account::g_accounts[userID];
      Gui::g_nextGui = GUI_EDITOR;
    } else (new Snackbar("No save file for this user available!"))->show();
  }

  if (kdown & KEY_ZL) {
    m_editableOnly = !m_editableOnly;
    m_selected.titleIndex = 0;
  }

  if (kdown & KEY_X) {
    if (batchClicked) {
      bool batchFailed = false;
      (new MessageBox("Are you sure you want to backup all saves\non this console?\nThis might take a while.", MessageBox::YES_NO))->setSelectionAction([&](s8 selection) {
      
        char backupName[33];
        time_t t = time(nullptr);
        std::stringstream initialText;
        initialText << std::put_time(std::gmtime(&t), "%Y%m%d_%H%M%S");        

        if (selection) {
          if(!Gui::requestKeyboardInput("Backup name", "Please enter a name for the backup to be saved under.", initialText.str(), SwkbdType_QWERTY, backupName, 32))
            return;

          s16 res;
          u16 failed_titles = 0;
          
          for (auto title : Title::g_titles) {
            for (u128 userID : Title::g_titles[title.first]->getUserIDs()) {
              if((res = backupSave(title.first, userID, true, backupName))) {
                batchFailed = true;
                failed_titles++;
              }
            }

            if (!batchFailed)
              (new Snackbar("Successfully created backups!"))->show();
            else {
              std::stringstream errorMessage;
              errorMessage << "Failed to backup " << failed_titles << " titles!";
              (new Snackbar(errorMessage.str()))->show();
            }
          }
        }
      })->show();
    }
    else {
      bool batchFailed = false;
      s16 res;

      time_t t = time(nullptr);
      char backupName[33];
      std::stringstream initialText;
      initialText << std::put_time(std::gmtime(&t), "%Y%m%d_%H%M%S");

      if (!Gui::requestKeyboardInput("Backup name", "Please enter a name for the backup to be saved under.", initialText.str(), SwkbdType_QWERTY, backupName, 32))
        return;

      for (u128 userID : Title::g_titles[m_selected.titleId]->getUserIDs()) {
        if((res = backupSave(m_selected.titleId, userID, true, backupName))) {
          batchFailed = true;
        }
      }

      if (!batchFailed)
        (new Snackbar("Successfully created backup!"))->show();
      else {
        switch(res) {
          case 1: (new Snackbar("Failed to mount save file!"))->show(); break;
          case 2: (new Snackbar("A backup with this name already exists!"))->show(); break;
          case 3: (new Snackbar("Failed to create backup!"))->show(); break;
        }
      }      
    }
  }

  if (kdown & KEY_MINUS) {
    (new MessageBox("Checking for updates...", MessageBox::NONE))->show();
    GuiMain::g_shouldUpdate = true;
  }

  batchClicked = (kdown & KEY_ZR) > 0;
}

void GuiMain::onTouch(touchPosition &touch) {
  if (Title::g_titles.size() == 0) return;

  u8 x = floor((touch.px + xOffset) / 256.0F);
  u8 y = floor(touch.py / 256.0F);
  u8 title = y + x * 2;

  if (y <= 1 && title < Title::g_titles.size()) {
    if (m_editableOnly && title > (m_editableCount - 1)) return;
      if (m_selected.titleIndex == title) {
        u128 userID = Gui::requestPlayerSelection();

        Title::g_currTitle = Title::g_titles[m_selected.titleId];
        std::vector<u128> users = Title::g_titles[m_selected.titleId]->getUserIDs();

        if(userID == 0x00)
          return;

        if (std::find(users.begin(), users.end(), userID) != users.end()) {
          Title::g_currTitle = Title::g_titles[m_selected.titleId];
          Account::g_currAccount = Account::g_accounts[userID];
          Gui::g_nextGui = GUI_EDITOR;
        } else (new Snackbar("No save file for this user available!"))->show();      
      }

      m_selected.titleIndex = title;
    }
}

void GuiMain::onGesture(touchPosition &startPosition, touchPosition &endPosition) {
  if (Title::g_titles.size() == 0) return;

}
