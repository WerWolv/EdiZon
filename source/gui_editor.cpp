#include "gui_editor.hpp"
#include "gui_main.hpp"

#include "title.hpp"
#include "save.hpp"

#include "widget_switch.hpp"
#include "widget_value.hpp"
#include "widget_list.hpp"

#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>
#include <math.h>
#include <unordered_map>
#include <algorithm>
#include <iostream>
#include <regex>

u8 *GuiEditor::g_currSaveFile = nullptr;
std::string GuiEditor::g_currSaveFileName = "";

u8* titleIcon;

std::vector<std::string> backupNames;
std::vector<std::string> saveFiles;

u16 widgetPage;
u16 widgetPageCnt;

bool hasConfigFile;

void updateSaveFileList(const char *path);

GuiEditor::GuiEditor() : Gui() {
  titleIcon = new u8[128*128*3];

  Gui::resizeImage(Title::g_currTitle->getTitleIcon(), titleIcon, 256, 256, 128, 128);

  widgetPage = 0;
  widgetPageCnt = 0;
  Widget::g_selectedWidgetIndex = 0;

  hasConfigFile = loadConfigFile(m_offsetFile);
}

GuiEditor::~GuiEditor() {
  for (auto it = m_widgets.begin(); it != m_widgets.end(); it++)
    delete it->widget;

  delete titleIcon;
  delete[] GuiEditor::g_currSaveFile;
  GuiEditor::g_currSaveFile = nullptr;
  GuiEditor::g_currSaveFileName = "";

  backupNames.clear();
  saveFiles.clear();
}

void GuiEditor::draw() {
  Gui::beginDraw();

  std::stringstream ss;
  ss << "0x" << std::setfill('0') << std::setw(16) << std::uppercase << std::hex << Title::g_currTitle->getTitleID();

  Gui::drawRectangle(0, 0, Gui::framebuffer_width, Gui::framebuffer_height, currTheme.backgroundColor);
  Gui::drawImage(0, 0, 128, 128, titleIcon, IMAGE_MODE_RGB24);
  Gui::drawImage(Gui::framebuffer_width - 128, 0, 128, 128, Account::g_currAccount->getProfileImage(), IMAGE_MODE_RGB24);
  Gui::drawShadow(0, 0, Gui::framebuffer_width, 128);

  Gui::drawTextAligned(font24, (Gui::framebuffer_width / 2), 10, currTheme.textColor, Title::g_currTitle->getTitleName().c_str(), ALIGNED_CENTER);
  Gui::drawTextAligned(font20, (Gui::framebuffer_width / 2), 45, currTheme.textColor, Title::g_currTitle->getTitleAuthor().c_str(), ALIGNED_CENTER);
  Gui::drawTextAligned(font20, (Gui::framebuffer_width / 2), 80, currTheme.textColor, ss.str().c_str(), ALIGNED_CENTER);

  Widget::drawWidgets(this, m_widgets, 150, widgetPage * WIDGETS_PER_PAGE, (widgetPage + 1) * WIDGETS_PER_PAGE);

  Gui::drawRectangle(50, Gui::framebuffer_height - 70, Gui::framebuffer_width - 100, 2, currTheme.textColor);

  if (GuiEditor::g_currSaveFileName == "") {
    Gui::drawTextAligned(font20, Gui::framebuffer_width - 100, Gui::framebuffer_height - 50, currTheme.textColor, "\x03 - Backup     \x04 - Restore     \x02 - Back", ALIGNED_RIGHT);
    Gui::drawTextAligned(font24, (Gui::framebuffer_width / 2), (Gui::framebuffer_height / 2), currTheme.textColor, hasConfigFile ? "No save file loaded. Press \x08 to select one." : "No editor JSON file found. Editing is disabled.", ALIGNED_CENTER);
  } else
    Gui::drawTextAligned(font20, Gui::framebuffer_width - 100, Gui::framebuffer_height - 50, currTheme.textColor, "\x03 - Apply changes     \x02 - Cancel", ALIGNED_RIGHT);

  if (m_widgets.size() > WIDGETS_PER_PAGE) {
    for (u8 page = 0; page < widgetPageCnt; page++) {
      Gui::drawRectangle((Gui::framebuffer_width / 2) - widgetPageCnt * 15 + page * 30 , 615, 20, 20, currTheme.separatorColor);
      if (page == widgetPage)
        Gui::drawRectangled((Gui::framebuffer_width / 2) - widgetPageCnt * 15 + page * 30 + 4, 619, 12, 12, currTheme.highlightColor);
    }

    Gui::drawTextAligned(font24, (Gui::framebuffer_width / 2) - widgetPageCnt * 15 - 30, 598, currTheme.textColor, "\x05", ALIGNED_CENTER);
    Gui::drawTextAligned(font24, (Gui::framebuffer_width / 2) + widgetPageCnt * 15 + 18, 598, currTheme.textColor, "\x06", ALIGNED_CENTER);

  }

  if (currListSelector != nullptr)
    currListSelector->draw();

  Gui::endDraw();
}

bool GuiEditor::loadConfigFile(json &j) {
  std::stringstream path;
  path << CONFIG_ROOT << std::setfill('0') << std::setw(sizeof(u64) * 2) << std::uppercase << std::hex << Title::g_currTitle->getTitleID() << ".json";

  std::ifstream file(path.str().c_str());

  m_widgets.clear();

  if (file.fail()) {
    printf("Opening editor JSON file failed!\n");
    return false;
  }

  try {
    file >> j;
  } catch (json::parse_error& e) {
		printf("Failed to parse JSON file.\n");
		return false;
	}

  return true;
}

void GuiEditor::createWidgets() {

if (m_offsetFile == nullptr) return;

for (auto item : m_offsetFile["items"]) {
  if (item["widget"]["type"] == "int")
    m_widgets.push_back({ item["name"], new WidgetValue(item["addressSize"], item["valueSize"], item["widget"]["minValue"], item["widget"]["maxValue"]) });
  else if (item["widget"]["type"] == "bool")
    m_widgets.push_back({ item["name"], new WidgetSwitch(item["addressSize"], item["valueSize"], item["widget"]["onValue"], item["widget"]["offValue"]) });
  else if(item["widget"]["type"] == "list")
    m_widgets.push_back({ item["name"], new WidgetList(this, item["addressSize"], item["valueSize"], item["widget"]["listItemNames"], item["widget"]["listItemValues"]) });

  m_widgets.back().widget->setOffset(strtol(item["indirectAddress"].get<std::string>().c_str(), 0, 16), strtol(item["address"].get<std::string>().c_str(), 0, 16));
}

  widgetPageCnt = ceil(m_widgets.size() / WIDGETS_PER_PAGE);
}

void updateBackupList() {
  DIR *dir;
  struct dirent *ent;

  std::stringstream path;
  path << "/EdiZon/" << std::setfill('0') << std::setw(16) << std::uppercase << std::hex << Title::g_currTitle->getTitleID();
  backupNames.clear();

  if ((dir = opendir(path.str().c_str())) != nullptr) {
    while ((ent = readdir(dir)) != nullptr)
      backupNames.push_back(ent->d_name);
    closedir(dir);
  }
}

void GuiEditor::updateSaveFileList(const char *saveFilePath) {
  DIR *dir;
  struct dirent *ent;
  FsFileSystem fs;

  if (m_offsetFile == nullptr) return;

  if (mountSaveByTitleAccountIDs(Title::g_currTitle->getTitleID(), Account::g_currAccount->getUserID(), fs))
    return;

  std::stringstream path;
  path << "save:" << saveFilePath;

  if ((dir = opendir(path.str().c_str())) != nullptr) {
    std::regex validSaveFileNames(m_offsetFile["files"].get<std::string>());

    while ((ent = readdir(dir)) != nullptr) {
      if (std::regex_match(ent->d_name, validSaveFileNames))
        saveFiles.push_back(std::string(saveFilePath) + ent->d_name);
    }

    std::sort(saveFiles.begin(), saveFiles.end());

    closedir(dir);
  }

  fsdevUnmountDevice(SAVE_DEV);
  fsFsClose(&fs);
}

void GuiEditor::onInput(u32 kdown) {
  if (Gui::currListSelector == nullptr) {

    if (kdown & KEY_MINUS) {
      if (!hasConfigFile) return;
      saveFiles.clear();

      for (auto saveFilePath : m_offsetFile["saveFilePaths"])
        updateSaveFileList(saveFilePath.get<std::string>().c_str());

      (new ListSelector(this, "Edit save file", "\x01 - Select      \x02 - Back", saveFiles))->setInputAction([&](u32 k, u16 selectedItem){
        if (k & KEY_A) {
          if (saveFiles.size() != 0) {
            size_t length;

            delete[] GuiEditor::g_currSaveFile;
            GuiEditor::g_currSaveFile = nullptr;
            GuiEditor::g_currSaveFileName = "";

            GuiEditor::g_currSaveFileName = saveFiles[Gui::currListSelector->selectedItem].c_str();

            if (loadSaveFile(&GuiEditor::g_currSaveFile, &length, Title::g_currTitle->getTitleID(), Account::g_currAccount->getUserID(), GuiEditor::g_currSaveFileName.c_str()) == 0)
              createWidgets();
            else {
              (new Snackbar(this, "Failed to load save file! Is it empty?"))->show();
              delete[] GuiEditor::g_currSaveFile;
              GuiEditor::g_currSaveFile = nullptr;
              GuiEditor::g_currSaveFileName = "";

              for (auto it = m_widgets.begin(); it != m_widgets.end(); it++)
                delete it->widget;

              m_widgets.clear();
            }
            Gui::currListSelector->hide();
          }
        }
      })->show();
    }

    if (GuiEditor::g_currSaveFileName != "") {
      if (kdown & KEY_B) {
        delete[] GuiEditor::g_currSaveFile;
        GuiEditor::g_currSaveFileName = "";
        GuiEditor::g_currSaveFile = nullptr;

        for (auto it = m_widgets.begin(); it != m_widgets.end(); it++)
          delete it->widget;

        m_widgets.clear();
        return;
      }

      if (kdown & KEY_X) {
        size_t length;
        if(!storeSaveFile(GuiEditor::g_currSaveFile, &length, Title::g_currTitle->getTitleID(), Account::g_currAccount->getUserID(), GuiEditor::g_currSaveFileName.c_str()))
          (new Snackbar(this, "Sucessfully injected modified values!"))->show();
        else
          (new Snackbar(this, "Injection of modified values failed!"))->show();

        delete[] GuiEditor::g_currSaveFile;
        GuiEditor::g_currSaveFile = nullptr;
        GuiEditor::g_currSaveFileName = "";

        for (auto it = m_widgets.begin(); it != m_widgets.end(); it++)
          delete it->widget;

        m_widgets.clear();
        return;
      }

      if (kdown & KEY_L) {
        if (widgetPage > 0)
          widgetPage--;
        Widget::g_selectedWidgetIndex = WIDGETS_PER_PAGE * widgetPage;
      }

      if (kdown & KEY_R) {
        if (widgetPage < widgetPageCnt - 1)
          widgetPage++;
        Widget::g_selectedWidgetIndex = WIDGETS_PER_PAGE * widgetPage ;
      }

      if (kdown & KEY_UP) {
        if (Widget::g_selectedWidgetIndex > 0)
          Widget::g_selectedWidgetIndex--;
        widgetPage = floor(Widget::g_selectedWidgetIndex / WIDGETS_PER_PAGE);
      }

      if (kdown & KEY_DOWN) {
        if (Widget::g_selectedWidgetIndex < m_widgets.size() - 1)
          Widget::g_selectedWidgetIndex++;
        widgetPage = floor(Widget::g_selectedWidgetIndex / WIDGETS_PER_PAGE);
      }
    } else {
      if (kdown & KEY_B) {
        Gui::g_nextGui = GUI_MAIN;
      }

      if (kdown & KEY_X) {
        s16 res;

        if(!(res = backupSave(Title::g_currTitle->getTitleID(), Account::g_currAccount->getUserID())))
          (new Snackbar(this, "Sucessfully created backup!"))->show();
        else (new Snackbar(this, "An error occured while creating the backup! Error " + std::to_string(res)))->show();
      }

      if (kdown & KEY_Y) {
        updateBackupList();

        (new ListSelector(this, "Restore Backup", "\x01 - Restore     \x03 - Delete      \x02 - Back", backupNames))->setInputAction([&](u32 k, u16 selectedItem){
          if (k & KEY_A) {
              if (backupNames.size() != 0) {
                s16 res;

                if(!(res = restoreSave(Title::g_currTitle->getTitleID(), Account::g_currAccount->getUserID(), backupNames[Gui::currListSelector->selectedItem].c_str())))
                  (new Snackbar(this, "Sucessfully loaded backup!"))->show();
                else (new Snackbar(this, "An error occured while restoring the backup! Error " + std::to_string(res)))->show();

                Gui::currListSelector->hide();
            }
          }

          if (k & KEY_X) {
            std::stringstream path;
            path << "/EdiZon/" << std::setfill('0') << std::setw(16) << std::uppercase << std::hex << Title::g_currTitle->getTitleID();
            path << "/" << backupNames[Gui::currListSelector->selectedItem];
            deleteDirRecursively(path.str().c_str(), false);
            updateBackupList();

            if (Gui::currListSelector->selectedItem == backupNames.size() && Gui::currListSelector->selectedItem > 0)
              Gui::currListSelector->selectedItem--;
          }
        })->show();
      }
    }
  } else if(kdown != 0) {
    Gui::currListSelector->onInput(kdown);
    return;
  }

    if(kdown != 0)
      Widget::handleInput(kdown, m_widgets);
}

void GuiEditor::onTouch(touchPosition &touch) {
  if (Gui::currListSelector == nullptr) {
    s8 widgetTouchPos = floor((touch.py - 150) / (static_cast<float>(WIDGET_HEIGHT) + WIDGET_SEPARATOR)) + WIDGETS_PER_PAGE * widgetPage;

    if (touch.px < 128 && touch.py < 128) {
      Title *nextTitle = nullptr;
      bool isCurrTitle = false;

      for (auto title : Title::g_titles) {
        if (isCurrTitle) {
          nextTitle = title.second;
          break;
        }

        isCurrTitle = title.second == Title::g_currTitle;
      }

      if (nextTitle == nullptr)
        nextTitle = Title::g_titles.begin()->second;

      Title::g_currTitle = nextTitle;
      Account::g_currAccount = Account::g_accounts[Title::g_currTitle->getUserIDs()[0]];
      Gui::g_nextGui = GUI_EDITOR;

    }

    if (touch.px > Gui::framebuffer_width - 128 && touch.py < 128) {
      Account *nextAccount = nullptr;
      bool isCurrAccount = false;

      for (auto userID : Title::g_currTitle->getUserIDs()) {
        if (isCurrAccount) {
          nextAccount = Account::g_accounts[userID];
          break;
        }
        isCurrAccount = userID == Account::g_currAccount->getUserID();
      }

      if (nextAccount == nullptr)
        nextAccount = Account::g_accounts[Title::g_currTitle->getUserIDs()[0]];

      if (Title::g_currTitle->getUserIDs().size() != 1) {
        Account::g_currAccount = nextAccount;
        Gui::g_nextGui = GUI_EDITOR;
      } else nextAccount = nullptr;
    }

    if (touch.px > 100 && touch.px < Gui::framebuffer_width - 100 && m_widgets.size() > 0) {
      if (widgetTouchPos >= 0 && widgetTouchPos < static_cast<u16>(m_widgets.size()) && widgetTouchPos < (WIDGETS_PER_PAGE * (widgetPage + 1)) - (widgetPage == widgetPageCnt ? static_cast<u16>(m_widgets.size()) % static_cast<u16>(WIDGETS_PER_PAGE + 1) : 0)) {
        if (Widget::g_selectedWidgetIndex == widgetTouchPos)
          m_widgets[widgetTouchPos].widget->onTouch(touch);
        Widget::g_selectedWidgetIndex = widgetTouchPos;
      }
    }
  }
}
