#include "guis/gui_main.hpp"

#include "helpers/save.hpp"
#include "helpers/title.hpp"
#include "helpers/editor_config_parser.hpp"
#include "helpers/account.hpp"

#include "beta_bin.h"
#include "edizon_logo_bin.h"

#include <string>
#include <sstream>
#include <math.h>
#include <stdlib.h>
#include <numeric>

#include "helpers/util.h"

static s64 xOffset, xOffsetNext;
static bool finishedDrawing = true;
static s64 startOffset = 0;
 
static color_t arrowColor;

GuiMain::GuiMain() : Gui() {
  updateEditableTitlesList();

  arrowColor = COLOR_WHITE;
}

GuiMain::~GuiMain() {

}

void GuiMain::update() {
  Gui::update();

  if (xOffset != xOffsetNext && finishedDrawing) {
    double deltaOffset = xOffsetNext - xOffset;
    double scrollSpeed = deltaOffset / 30.0F;

    if (xOffsetNext > xOffset)
      xOffset += ceil((abs(deltaOffset) > scrollSpeed) ? scrollSpeed : deltaOffset);
    else
      xOffset += floor((abs(deltaOffset) > scrollSpeed) ? scrollSpeed : deltaOffset);

    startOffset = xOffsetNext;
  }

  if (xOffset > 0)
    arrowColor.a = arrowColor.a > 0 ? arrowColor.a - 1 : 0;
  else 
    arrowColor.a = arrowColor.a < 0xFF ? arrowColor.a + 1 : 0xFF;
}

void GuiMain::draw() {
  s64 x = 0, y = 32, currItem = 0;
  s64 selectedX = 0, selectedY = 0;
  bool tmpEditableOnly = m_editableOnly;
  static u32 splashCnt = 0;

  Gui::beginDraw();

  finishedDrawing = false;

  #if SPLASH_ENABLED

    if (!Gui::g_splashDisplayed) {
      Gui::drawRectangle(0, 0, Gui::g_framebuffer_width, Gui::g_framebuffer_height, Gui::makeColor(0x5D, 0x4F, 0x4E, 0xFF));
      Gui::drawImage(Gui::g_framebuffer_width / 2 - 128, Gui::g_framebuffer_height / 2 - 128, 256, 256, edizon_logo_bin, IMAGE_MODE_BGR24);

      // if (splashCnt++ >= 70)
        Gui::g_splashDisplayed = true;

      Gui::endDraw();
      return;
    }
    
  #endif
  
  Gui::drawRectangle(0, 0, Gui::g_framebuffer_width, Gui::g_framebuffer_height, currTheme.backgroundColor);

  if (Title::g_titles.size() == 0) {
    Gui::drawTextAligned(font24, (Gui::g_framebuffer_width / 2), (Gui::g_framebuffer_height / 2), currTheme.textColor, "No games or saves found on this system! Please press \uE0E1 to exit EdiZon!", ALIGNED_CENTER);
    Gui::endDraw();
    return;
  }

  if ((m_editableOnly ? EditorConfigParser::g_editableTitles.size() : Title::g_titles.size()) > 10) {
    Gui::drawRectangle(0, 544, Gui::g_framebuffer_width, 5, currTheme.tooltipColor);

    u32 scrollbarPos = (static_cast<double>(xOffset) / (std::ceil(std::round(m_editableOnly ? EditorConfigParser::g_editableTitles.size() : Title::g_titles.size()) / 4.0F) * 2 * 256)) * Gui::g_framebuffer_width;
    u32 scrollbarWidth = static_cast<double>(Gui::g_framebuffer_width) / ((std::round((m_editableOnly ? EditorConfigParser::g_editableTitles.size() : Title::g_titles.size()) / 2.0F) * 2) / 10.0F);

    Gui::drawRectangle(scrollbarPos, 544, scrollbarWidth, 5, currTheme.tooltipTextColor);
  }

  for (auto title : Title::g_titles) {
    if (currItem == m_selected.titleIndex) {
      selectedX = x - xOffset;
      selectedY = y;
      m_selected.titleId = title.first;
    }

    if (!tmpEditableOnly || EditorConfigParser::g_editableTitles.count(title.first)) {
      if (x - xOffset >= -256 && x - xOffset < Gui::g_framebuffer_width) {
        Gui::drawImage(x - xOffset, y, 256, 256, title.second->getTitleIcon(), IMAGE_MODE_RGB24);

        if (EditorConfigParser::g_betaTitles[title.first])
          Gui::drawImage(x - xOffset, y, 150, 150, 256, 256, beta_bin, IMAGE_MODE_ABGR32);

        if (y == 320 || title.first == (--Title::g_titles.end())->first)
          Gui::drawShadow(x - xOffset, y, 256, 256);
        
        if (title.first == Title::g_activeTitle) {
          Gui::drawRectangled(x - xOffset, y, 256, 256, Gui::makeColor(0x30, 0x30, 0x30, 0xA0));
          Gui::drawTextAligned(fontTitle, x - xOffset + 245, y + 250, currTheme.selectedColor, "\uE12C", ALIGNED_RIGHT);
        }
      }

      y = y == 32 ? 288 : 32;
      x = floor(++currItem / 2.0F) * 256;
    }
  }

  Gui::drawRectangle(0, 0, Gui::g_framebuffer_width, 32, currTheme.selectedButtonColor);
  Gui::drawShadow(0, 0, Gui::g_framebuffer_width, 32);

  char timeBuffer[6];
  char batteryBuffer[5];
  getCurrTimeString(timeBuffer);
  getCurrBatteryPercentage(batteryBuffer);

  Gui::drawTextAligned(font14, Gui::g_framebuffer_width - 8, 3, currTheme.separatorColor, timeBuffer, ALIGNED_RIGHT);
  Gui::drawTextAligned(font14, Gui::g_framebuffer_width - 80, 3, currTheme.separatorColor, batteryBuffer, ALIGNED_RIGHT);
  Gui::drawTextAligned(font14, 8, 3, currTheme.separatorColor, "EdiZon v" VERSION_STRING, ALIGNED_LEFT);

  Gui::drawRectangled(Gui::g_framebuffer_width - 72, 5, 7, 18, currTheme.separatorColor);
  Gui::drawRectangled(Gui::g_framebuffer_width - 75, 8, 13, 18, currTheme.separatorColor);

  if (tmpEditableOnly && EditorConfigParser::g_editableTitles.size() == 0) {
    Gui::drawTextAligned(font24, (Gui::g_framebuffer_width / 2), (Gui::g_framebuffer_height / 2), currTheme.textColor, "No editable games found on this system!", ALIGNED_CENTER);
    Gui::endDraw();
    return;
  }
  else {
    if (m_selected.titleIndex != -1 && m_selected.extraOption == -1) {
      Gui::drawRectangled(selectedX - 5, selectedY - 5, 266, 266, currTheme.highlightColor);
      Gui::drawImage(selectedX, selectedY, 256, 256, Title::g_titles[m_selected.titleId]->getTitleIcon(), IMAGE_MODE_RGB24);

      if (EditorConfigParser::g_betaTitles[m_selected.titleId])
        Gui::drawImage(selectedX, selectedY, 150, 150, 256, 256, beta_bin, IMAGE_MODE_ABGR32);

      if (m_selected.titleId == Title::g_activeTitle) {
        Gui::drawRectangled(selectedX, selectedY, 256, 256, Gui::makeColor(0x30, 0x30, 0x30, 0xA0));
        Gui::drawTextAligned(fontTitle, selectedX + 245, selectedY + 250, currTheme.selectedColor, "\uE12C", ALIGNED_RIGHT);
      }

      Gui::drawShadow(selectedX - 5, selectedY - 5, 266, 266);
    }

    if (m_selected.extraOption != -1) {
      Gui::drawRectangled(455 + 150 * m_selected.extraOption, 557, 70, 70, currTheme.highlightColor);
    }

    Gui::drawRectangled(458, 560, 64, 64, currTheme.selectedButtonColor);
    Gui::drawRectangled(608, 560, 64, 64, currTheme.selectedButtonColor);
    Gui::drawRectangled(758, 560, 64, 64, currTheme.selectedButtonColor);

    Gui::drawTextAligned(fontTitle, 469, 608, Gui::makeColor(0x11, 0x75, 0xFB, 0xFF), "\uE02B", ALIGNED_LEFT);
    Gui::drawTextAligned(fontTitle, 619, 608, Gui::makeColor(0x34, 0xA8, 0x53, 0xFF), "\uE02E", ALIGNED_LEFT);
    Gui::drawTextAligned(fontTitle, 769, 608, Gui::makeColor(0xEA, 0x43, 0x35, 0xFF), "\uE017", ALIGNED_LEFT);


    Gui::drawRectangle((u32)((Gui::g_framebuffer_width - 1220) / 2), Gui::g_framebuffer_height - 73, 1220, 1, currTheme.textColor);

    if (m_selected.extraOption == 0)
      Gui::drawTextAligned(font14, 490, 623, currTheme.tooltipTextColor, "Cheats", ALIGNED_CENTER);
    else if (m_selected.extraOption == 1)
      Gui::drawTextAligned(font14, 640, 623, currTheme.tooltipTextColor, "Guide", ALIGNED_CENTER);
    else if (m_selected.extraOption == 2)
      Gui::drawTextAligned(font14, 790, 623, currTheme.tooltipTextColor, "About", ALIGNED_CENTER);

    std::string buttonHintStr = "";

    buttonHintStr  = !tmpEditableOnly ? "\uE0E6 Editable titles     " : "\uE0E6 All titles     ";
    buttonHintStr += m_backupAll ? "(\uE0E7) + \uE0E2 Backup all     " : "(\uE0E7) + \uE0E2 Backup     ";
    buttonHintStr += "\uE0E1 Back     \uE0E0 OK";

    Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 50, Gui::g_framebuffer_height - 50, currTheme.textColor, buttonHintStr.c_str(), ALIGNED_RIGHT);

    if (m_selected.titleIndex != -1)
      Gui::drawTooltip(selectedX + 128, 288, Title::g_titles[m_selected.titleId]->getTitleName().c_str(), currTheme.tooltipColor, currTheme.tooltipTextColor, 0xFF, m_selected.titleIndex % 2);
  }

  finishedDrawing = true;

  Gui::endDraw();
}

void GuiMain::onInput(u32 kdown) {
  if (kdown & KEY_B)
    Gui::g_requestExit = true;

  if (Title::g_titles.size() == 0) return;

  if (m_selected.extraOption == -1) { /* one of the titles is selected */
    if (kdown & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT | KEY_A | KEY_X)) {
      if (m_selected.titleIndex == -1 || (m_selected.titleIndex / 2 + 1) * 256 < xOffset || (m_selected.titleIndex / 2) * 256 > xOffset + 6 * 256) {
        m_selected.titleIndex = std::ceil(xOffset / 256.0F) * 2;
        return;
      }
    }

    if (kdown & KEY_LEFT) {
      if (static_cast<s16>(m_selected.titleIndex - 2) >= 0)
        m_selected.titleIndex -= 2;
    } else if (kdown & KEY_RIGHT) {
      if (static_cast<u16>(m_selected.titleIndex + 2) < ((!m_editableOnly) ?  Title::g_titles.size() : EditorConfigParser::g_editableTitles.size()))
        m_selected.titleIndex += 2;
    } else if (kdown & KEY_UP) {
      if ((m_selected.titleIndex % 2) == 1) {
            m_selected.titleIndex--;
      }
    } else if (kdown & KEY_DOWN) {
      if ((m_selected.titleIndex % 2) == 0) {
        if (static_cast<u16>(m_selected.titleIndex + 1) < ((!m_editableOnly) ?  Title::g_titles.size() : EditorConfigParser::g_editableTitles.size()))
          m_selected.titleIndex++;
      } else {
        if (m_selected.titleIndex < (std::ceil(xOffset / 256.0F) * 2 + 4))
          m_selected.extraOption = 0;
        else if (m_selected.titleIndex < (std::ceil(xOffset / 256.0F) * 2 + 6))
          m_selected.extraOption = 1;
        else 
          m_selected.extraOption = 2;

        m_selected.titleIndex = -1;
      } 
    }

    if (kdown & KEY_A) {
      if (m_selected.titleId == Title::g_activeTitle) {
        (new Snackbar("The save files of a running game cannot be accessed."))->show();
        return;
      }
      AccountUid userID = Gui::requestPlayerSelection();
      std::vector<AccountUid> users = Title::g_titles[m_selected.titleId]->getUserIDs();

      if(userID.uid[0] == 0x00 && userID.uid[1] == 0x00)
        return;

      if (std::find(users.begin(), users.end(), userID) != users.end()) {
        Title::g_currTitle = Title::g_titles[m_selected.titleId];
        Account::g_currAccount = Account::g_accounts[userID];
        Gui::g_nextGui = GUI_EDITOR;
      } else (new Snackbar("No save file for this user available!"))->show();
    }

    if (kdown & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT)) {
      if (m_selected.titleIndex != -1) {
        if (m_selected.titleIndex / 2 - (xOffset / 256) > 3)
          xOffsetNext = std::min(static_cast<u32>((m_selected.titleIndex / 2 - 3) * 256), static_cast<u32>(std::ceil(((!m_editableOnly) ?  Title::g_titles.size() : EditorConfigParser::g_editableTitles.size()) / 2.0F - 5) * 256));

        if (m_selected.titleIndex / 2 - (xOffset / 256) < 1)
          xOffsetNext = std::max((m_selected.titleIndex / 2 - 1) * 256, 0);
      }
    }

    if (kdown & KEY_X) {
      if (m_selected.titleId == Title::g_activeTitle) {
        (new Snackbar("The save files of a running game cannot be accessed."))->show();
        return;
      }

      if (m_backupAll) {
        (new MessageBox("Are you sure you want to backup all saves \n on this console? \n This might take a while.", MessageBox::YES_NO))->setSelectionAction([&](s8 selection) {
          bool batchFailed = false;

          char backupName[65];
          time_t t = time(nullptr);
          std::stringstream initialText;
          initialText << std::put_time(std::gmtime(&t), "%Y%m%d_%H%M%S");        

          if (selection) {
            if(!Gui::requestKeyboardInput("Backup name", "Please enter a name for the backup to be saved under.", initialText.str(), SwkbdType_QWERTY, backupName, 32))
              return;

            (new MessageBox("Creating batch backup. \n \n This might take a while...", MessageBox::NONE))->show();
            requestDraw();

            s16 res;
            u16 failed_titles = 0;
            
            for (auto title : Title::g_titles) {
              for (AccountUid userID : Title::g_titles[title.first]->getUserIDs()) {
                if((res = backupSave(title.first, userID, true, backupName))) {
                  batchFailed = true;
                  failed_titles++;
                }
              }

              Gui::g_currMessageBox->hide();

              if (!batchFailed)
                (new Snackbar("Successfully created backups!"))->show();
              else {
                std::stringstream errorMessage;
                errorMessage << "Failed to backup " << failed_titles << " titles!";
                (new Snackbar(errorMessage.str()))->show();
              }
            }
                    
            Gui::g_currMessageBox->hide();
          } else Gui::g_currMessageBox->hide();
        })->show();
      }
      else {
        bool batchFailed = false;
        s16 res;

        time_t t = time(nullptr);
        char backupName[65];
        std::stringstream initialText;
        initialText << std::put_time(std::gmtime(&t), "%Y%m%d_%H%M%S");

        if (!Gui::requestKeyboardInput("Backup name", "Please enter a name for the backup to be saved under.", initialText.str(), SwkbdType_QWERTY, backupName, 32))
          return;

        for (AccountUid userID : Title::g_titles[m_selected.titleId]->getUserIDs()) {
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
  } else { /* One of the extra options (Cheats, Tutorial or Credits) is selected */
    if (kdown & KEY_UP) {
      m_selected.titleIndex = std::min(static_cast<u32>(std::ceil(xOffset / 256.0F) * 2 + 2 * m_selected.extraOption + 3), static_cast<u32>(((!m_editableOnly) ?  Title::g_titles.size() : EditorConfigParser::g_editableTitles.size()) - 1));
      m_selected.extraOption = -1;
    }
    else if (kdown & KEY_LEFT) {
      if (m_selected.extraOption > 0)
        m_selected.extraOption--;
    } else if (kdown & KEY_RIGHT) {
      if (m_selected.extraOption < 2)
        m_selected.extraOption++;
    }

    if (kdown & KEY_A) {
      switch(m_selected.extraOption) {
        case 0:
          Gui::g_nextGui = GUI_CHEATS;
          break;
        case 1:
          Gui::g_nextGui = GUI_GUIDE;
          break;
        case 2:
          Gui::g_nextGui = GUI_ABOUT;
          break;
        default: break;
      }
    }
  }    

  if (kdown & KEY_ZL) {
    m_editableOnly = !m_editableOnly;
    m_selected.titleIndex = 0;
    xOffsetNext = 0;
  }

  m_backupAll = (kdown & KEY_ZR) > 0;
}

void GuiMain::onTouch(touchPosition &touch) {
  if (((!m_editableOnly) ?  Title::g_titles.size() : EditorConfigParser::g_editableTitles.size()) == 0) return;

  u8 x = floor((touch.px + xOffset) / 256.0F);
  u8 y = floor((touch.py - 32) / 256.0F);
  u8 title = y + x * 2;

  if (touch.py < 32) return;

  if (y < 2) {
    if (title < ((!m_editableOnly) ?  Title::g_titles.size() : EditorConfigParser::g_editableTitles.size())) {
      if (m_editableOnly && title > (EditorConfigParser::g_editableTitles.size() - 1)) return;
      
      if (m_selected.titleIndex == title) {
        if (m_selected.titleId == Title::g_activeTitle) {
          (new Snackbar("The save files of a running game cannot be accessed."))->show();
          return;
        }

        AccountUid userID = Gui::requestPlayerSelection();

        Title::g_currTitle = Title::g_titles[m_selected.titleId];
        std::vector<AccountUid> users = Title::g_titles[m_selected.titleId]->getUserIDs();

        if(userID.uid[0] == 0x00 && userID.uid[1] == 0x00)
          return;

        if (std::find(users.begin(), users.end(), userID) != users.end()) {
          Title::g_currTitle = Title::g_titles[m_selected.titleId];
          Account::g_currAccount = Account::g_accounts[userID];
          Gui::g_nextGui = GUI_EDITOR;
        } else (new Snackbar("No save file available for this user!"))->show();      
      }

      m_selected.titleIndex = title;
      m_selected.extraOption = -1;
    }
  } else {
    if (touch.py > 560 && touch.py < 624) {
      if (touch.px > 458 && touch.px < 522) { // Touched cheats button
        if (m_selected.extraOption == 0)
          Gui::g_nextGui = GUI_CHEATS;
        else {
          m_selected.extraOption = 0;
          m_selected.titleIndex = -1;
        }
      } else if (touch.px > 608 && touch.px < 672) { // Touched guide button
        if (m_selected.extraOption == 1)
          Gui::g_nextGui = GUI_GUIDE;
        else {
          m_selected.extraOption = 1;
          m_selected.titleIndex = -1;
        }
      } else if (touch.px > 758 && touch.px < 822) { // Touched information button
        if (m_selected.extraOption == 2)
          Gui::g_nextGui = GUI_ABOUT;
        else {
          m_selected.extraOption = 2;
          m_selected.titleIndex = -1;
        }
      }
    }
  }
}

inline s8 sign(s32 value) {
  return (value > 0) - (value < 0); 
}

void GuiMain::onGesture(touchPosition startPosition, touchPosition currPosition, bool finish) {
  static std::vector<s32> positions;
  static touchPosition oldPosition;

  m_selected.titleIndex = -1;
  m_selected.extraOption = -1;

  if (((!m_editableOnly) ?  Title::g_titles.size() : EditorConfigParser::g_editableTitles.size()) == 0) return;

  if (finish) {
    s32 velocity = (std::accumulate(positions.begin(), positions.end(), 0) / static_cast<s32>(positions.size())) * 2;
    
    xOffsetNext = std::min(std::max<s32>(xOffset + velocity * 1.5F, 0), 256 * static_cast<s32>(std::ceil(((!m_editableOnly) ?  Title::g_titles.size() : EditorConfigParser::g_editableTitles.size()) / 2.0F - 5)));

    startOffset = xOffsetNext;

    positions.clear();
    oldPosition = {0};
  }
  else {
    xOffset = startOffset + (static_cast<s32>(startPosition.px) - static_cast<s32>(currPosition.px));
    xOffset = std::min(std::max<s32>(xOffset, 0), 256 * static_cast<s32>(std::ceil(((!m_editableOnly) ?  Title::g_titles.size() : EditorConfigParser::g_editableTitles.size()) / 2.0F - 5)));
    xOffsetNext = xOffset;
  }

  if (oldPosition.px != 0x00) {
    s32 pos = static_cast<s32>(oldPosition.px) - static_cast<s32>(currPosition.px);
    if (std::abs(pos) < 400)
      positions.push_back(pos);
  }

  if (positions.size() > 10)
    positions.erase(positions.begin());

  oldPosition = currPosition;
}

void GuiMain::updateEditableTitlesList() {
  EditorConfigParser::g_editableTitles.clear();
  EditorConfigParser::g_betaTitles.clear();

  for (auto title : Title::g_titles) {
    if (EditorConfigParser::hasConfig(title.first) == 0) {
      EditorConfigParser::g_editableTitles.insert({title.first, true});
    }
  }
}
