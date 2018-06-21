#include "gui_editor.hpp"
#include "gui_main.hpp"

#include "title.hpp"
#include "save.hpp"

#include "widget_switch.hpp"
#include "widget_value.hpp"

#include <string>
#include <sstream>
#include <map>
#include <math.h>

u8* titleIcon;

bool isRestoreListShown;
u16 selectedBackup;
std::vector<std::string> backupNames;

u16 widgetPage;
u16 widgetPageCnt;

const char* noItems = "No widgets to show.";

GuiEditor::GuiEditor() : Gui() {
  titleIcon = (u8*) malloc(128*128*3);

  Gui::resizeImage(Title::g_currTitle->getTitleIcon(), titleIcon, 256, 256, 128, 128);

  isRestoreListShown = false;
  selectedBackup = 0;
  widgetPage = 0;
  widgetPageCnt = ceil(m_widgets.size() / 5.0F);
  Widget::g_selectedWidgetIndex = 0;
  Widget::getList(m_widgets, m_files);
}

GuiEditor::~GuiEditor() {
  for(auto widget : m_widgets)
    delete[] widget.widget;

  for (auto& item : m_files)
  {
    printf("Destroying buffer.\n");
    //writeSaveFile(item, Title::g_currTitle->getTitleID(), Account::g_currAccount->getUserID());
    delete[] std::get<u8*>(item);
  }
}

void GuiEditor::draw() {
  Gui::beginDraw();

  std::stringstream ss;
  ss << "0x" << std::setfill('0') << std::setw(16) << std::hex << Title::g_currTitle->getTitleID();

  Gui::drawRectangle(0, 0, Gui::framebuffer_width, Gui::framebuffer_height, currTheme.backgroundColor);
  Gui::drawImage(0, 0, 128, 128, titleIcon, IMAGE_MODE_RGB24);
  Gui::drawImage(Gui::framebuffer_width - 128, 0, 128, 128, Account::g_currAccount->getProfileImage(), IMAGE_MODE_RGB24);
  Gui::drawShadow(0, 0, Gui::framebuffer_width, 128);

  u32 textWidth, textHeight;
  Gui::getTextDimensions(font24, Title::g_currTitle->getTitleName().c_str(), &textWidth, &textHeight);
  Gui::drawText(font24, (Gui::framebuffer_width / 2) - (textWidth / 2), 10, currTheme.textColor, Title::g_currTitle->getTitleName().c_str());
  Gui::getTextDimensions(font20, Title::g_currTitle->getTitleAuthor().c_str(), &textWidth, &textHeight);
  Gui::drawText(font20, (Gui::framebuffer_width / 2) - (textWidth / 2), 45, currTheme.textColor, Title::g_currTitle->getTitleAuthor().c_str());
  Gui::getTextDimensions(font20, ss.str().c_str(), &textWidth, &textHeight);
  Gui::drawText(font20, (Gui::framebuffer_width / 2) - (textWidth / 2), 80, currTheme.textColor, ss.str().c_str());

  if (m_widgets.size() == 0)
  {
  Gui::getTextDimensions(font20, noItems, &textWidth, &textHeight);
  Gui::drawText(font24, (Gui::framebuffer_width / 2) - (textWidth / 2), (Gui::framebuffer_height / 2) - (textHeight / 2), currTheme.textColor, noItems);
  } else
    Widget::drawWidgets(this, m_widgets, 150, widgetPage * 6, widgetPage * 6 + 6);

  for(u8 page = 0; page < widgetPageCnt; page++) {
    Gui::drawRectangle((Gui::framebuffer_width / 2) - ((40 * widgetPageCnt) / 2) + (40 * page), 615, 20, 20, currTheme.separatorColor);
    if(page == widgetPage)
      Gui::drawRectangled((Gui::framebuffer_width / 2) - ((40 * widgetPageCnt) / 2) + (40 * page) + 4, 619, 12, 12, currTheme.highlightColor);
  }

  Gui::drawRectangle(50, Gui::framebuffer_height - 70, Gui::framebuffer_width - 100, 2, currTheme.textColor);
  Gui::drawText(font20, 750, Gui::framebuffer_height - 50, currTheme.textColor, "X - Backup     Y - Restore     B - Back");

  if(isRestoreListShown) {
    Gui::drawRectangled(0, 0, Gui::framebuffer_width, Gui::framebuffer_height - 100, Gui::makeColor(0x00, 0x00, 0x00, 0xAA));
    Gui::drawRectangle(0, 220, Gui::framebuffer_width, Gui::framebuffer_height - 120, currTheme.backgroundColor);
    Gui::drawRectangle(50, 300, Gui::framebuffer_width - 100, 2, currTheme.textColor);
    Gui::drawText(font24, 100, 240, currTheme.textColor, "Restore backup");

    if(backupNames.size() != 0) {
      for(s16 currBackup = -2; currBackup < 3; currBackup++) {
        if((currBackup + selectedBackup) >= 0 && (currBackup + selectedBackup) < (s16)backupNames.size()) {
          Gui::drawText(font20, 300, 340 + 60 * (currBackup + 2), currTheme.textColor, backupNames[(currBackup + selectedBackup)].c_str());
          Gui::drawRectangle(250, 325 + 60 * (currBackup + 2), Gui::framebuffer_width - 500, 1, currTheme.separatorColor);
          Gui::drawRectangle(250, 325 + 60 * (currBackup + 3), Gui::framebuffer_width - 500, 1, currTheme.separatorColor);
        }
      }
      Gui::drawRectangled(245, 320 + 60 * 2, Gui::framebuffer_width - 490, 71, currTheme.highlightColor);
      Gui::drawRectangle(250, 325 + 60 * 2, Gui::framebuffer_width - 500, 61, currTheme.selectedButtonColor);
      Gui::drawText(font20, 300, 340 + 60 * 2, currTheme.textColor, backupNames[selectedBackup].c_str());
      Gui::drawShadow(245, 320 + 60 * 2, Gui::framebuffer_width - 491, 71);
    } else Gui::drawText(font20, 300, 340 + 60 * 2, currTheme.textColor, "No backups present!");

    Gui::drawRectangle(50, Gui::framebuffer_height - 70, Gui::framebuffer_width - 100, 2, currTheme.textColor);
    Gui::drawText(font20, 750, Gui::framebuffer_height - 50, currTheme.textColor, "A - Restore     X - Delete      B - Back");
  }

  Gui::endDraw();
}

void updateBackupList() {
  DIR *dir;
  struct dirent *ent;

  std::stringstream path;
  path << "/EdiZon/";
  path << std::setfill('0') << std::setw(16) << std::hex << Title::g_currTitle->getTitleID();
  backupNames.clear();
  if((dir = opendir(path.str().c_str())) != nullptr) {
    while((ent = readdir(dir)) != nullptr)
      backupNames.push_back(ent->d_name);
    closedir(dir);
  }
}

void GuiEditor::onInput(u32 kdown) {
  Widget::handleInput(kdown, m_widgets);

  if(isRestoreListShown) {
    if(kdown & KEY_A) {
        if(backupNames.size() != 0) {
          s16 res;
          if(!(res = restoreSave(Title::g_currTitle->getTitleID(), Account::g_currAccount->getUserID(), backupNames[selectedBackup].c_str())))
            (new Snackbar(this, "Sucessfully loaded backup!"))->show();
          else (new Snackbar(this, "An error occured while restoring the backup! Error " + std::to_string(res)))->show();

          isRestoreListShown = false;
      }
    }

    if(kdown & KEY_B)
      isRestoreListShown = false;

    if(kdown & KEY_X) {
      std::stringstream path;
      path << "/EdiZon/";
      path << std::setfill('0') << std::setw(16) << std::hex << Title::g_currTitle->getTitleID();
      path << "/" << backupNames[selectedBackup];
      deleteDirRecursively(path.str().c_str(), false);
      updateBackupList();

      if(selectedBackup == backupNames.size() && selectedBackup > 0)
        selectedBackup--;
    }

    if(kdown & KEY_UP)
      if(selectedBackup > 0)
        selectedBackup--;

    if(kdown & KEY_DOWN)
      if(selectedBackup < ((s16)backupNames.size() - 1))
        selectedBackup++;
  } else {
    if(kdown & KEY_B)
      Gui::g_nextGui = GUI_MAIN;

    if(kdown & KEY_X) {
      s16 res;
      if(!(res = backupSave(Title::g_currTitle->getTitleID(), Account::g_currAccount->getUserID())))
        (new Snackbar(this, "Sucessfully created backup!"))->show();
      else (new Snackbar(this, "An error occured while creating the backup! Error " + std::to_string(res)))->show();
    }

    if(kdown & KEY_Y) {
      isRestoreListShown = true;
      selectedBackup = 0;
      updateBackupList();
    }

    if(kdown & KEY_L) {
      if(widgetPage > 0)
        widgetPage--;
      Widget::g_selectedWidgetIndex = 6 * widgetPage;
    }

    if(kdown & KEY_R) {
      if(widgetPage < widgetPageCnt - 1)
        widgetPage++;
      Widget::g_selectedWidgetIndex = 6 * widgetPage;
    }

    if(kdown & KEY_UP) {
      if(Widget::g_selectedWidgetIndex > 0)
        Widget::g_selectedWidgetIndex--;
      widgetPage = floor(Widget::g_selectedWidgetIndex / 6.0F);
    }

    if(kdown & KEY_DOWN) {
      if(Widget::g_selectedWidgetIndex < m_widgets.size() - 1)
        Widget::g_selectedWidgetIndex++;
      widgetPage = floor(Widget::g_selectedWidgetIndex / 6.0F);
    }
  }
}

void GuiEditor::onTouch(touchPosition &touch) {
  if(!isRestoreListShown) {
    s8 widgetTouchPos = floor((touch.py - 150) / ((float)WIDGET_HEIGHT + WIDGET_SEPARATOR)) + 6 * widgetPage;

    if(touch.px < 256 && touch.py < 256) {
      Title *nextTitle = nullptr;
      bool isCurrTitle = false;
      for(auto title : Title::g_titles) {
        if(isCurrTitle) {
          nextTitle = title.second;
          break;
        }

        isCurrTitle = title.second == Title::g_currTitle;
      }
      if(nextTitle == nullptr)
        nextTitle = Title::g_titles.begin()->second;

      Title::g_currTitle = nextTitle;
      Account::g_currAccount = Account::g_accounts[Title::g_currTitle->getUserIDs()[0]];
      Gui::g_nextGui = GUI_EDITOR;

    }

    if(touch.px > Gui::framebuffer_width - 256 && touch.py < 256) {
      Account *nextAccount = nullptr;
      bool isCurrAccount = false;
      for(auto userID : Title::g_currTitle->getUserIDs()) {
        if(isCurrAccount) {
          nextAccount = Account::g_accounts[userID];
          break;
        }

        isCurrAccount = userID == Account::g_currAccount->getUserID();
      }
      if(nextAccount == nullptr)
        nextAccount = Account::g_accounts[Title::g_currTitle->getUserIDs()[0]];

      Account::g_currAccount = nextAccount;
      Gui::g_nextGui = GUI_EDITOR;

    }

    if(touch.px > 100 && touch.px < Gui::framebuffer_width - 100 && m_widgets.size() > 0) {
      if(widgetTouchPos >= 0 && widgetTouchPos < (s16)m_widgets.size() && widgetTouchPos < (6 * (widgetPage + 1)) - (widgetPage == widgetPageCnt ? (s16)m_widgets.size() % 6 + 1 : 0)) {
        if(Widget::g_selectedWidgetIndex == widgetTouchPos)
          m_widgets[widgetTouchPos].widget->onTouch(touch);
        Widget::g_selectedWidgetIndex = widgetTouchPos;
      }
    }
  }
}
