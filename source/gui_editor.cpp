#include "gui_editor.hpp"
#include "gui_main.hpp"

#include "title.hpp"
#include "save.hpp"

#include "widget_switch.hpp"
#include "widget_value.hpp"
#include "widget_list.hpp"

#include "lua_save_parser.hpp"

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
#include <iterator>

#define COLOR_THRESHOLD 35

u8 *GuiEditor::g_currSaveFile = nullptr;
std::string GuiEditor::g_currSaveFileName = "";

u8* titleIcon;

std::vector<std::string> backupNames;
std::vector<std::string> saveFiles;
u16 widgetPage;
std::map<std::string, u16> widgetPageCnt;

color_t dominantColor;
color_t textColor;

bool hasConfigFile;

LuaSaveParser luaParser;

void updateSaveFileList(const char *path);

GuiEditor::GuiEditor() : Gui() {
  titleIcon = new u8[128*128*3];
  u8 *smallTitleIcon = new u8[32*32*3];
  std::map<u32, u16> colors;

  dominantColor = Gui::makeColor(0xA0, 0xA0, 0xA0, 0xFF);

  Gui::resizeImage(Title::g_currTitle->getTitleIcon(), titleIcon, 256, 256, 128, 128);
  Gui::resizeImage(Title::g_currTitle->getTitleIcon(), smallTitleIcon, 256, 256, 32, 32);

  for (u16 i = 0; i < 32 * 32 * 3; i += 3) {
    u32 currColor = smallTitleIcon[i + 0] << 16 | smallTitleIcon[i + 1] << 8 | smallTitleIcon[i + 2];
    colors[currColor]++;
  }

  u32 dominantUseCnt = 0;
  for (auto [color, count] : colors) {
    if (count > dominantUseCnt) {
      color_t colorCandidate = Gui::makeColor((color & 0xFF0000) >> 16, (color & 0x00FF00) >> 8, (color & 0x0000FF), 0xFF);

      if(!(abs(static_cast<s16>(colorCandidate.r) - colorCandidate.g) > COLOR_THRESHOLD || abs(static_cast<s16>(colorCandidate.r) - colorCandidate.b) > COLOR_THRESHOLD))
        continue;

      dominantUseCnt = count;
      dominantColor = colorCandidate;
    }
  }

  textColor = (dominantColor.r > 0x80 && dominantColor.g > 0x80 && dominantColor.b > 0x80) ? COLOR_BLACK : COLOR_WHITE;

  widgetPage = 0;
  Widget::g_selectedWidgetIndex = 0;
  Widget::g_selectedCategory = "";

  hasConfigFile = loadConfigFile(m_offsetFile);

}

GuiEditor::~GuiEditor() {
  for (auto const& [category, widgets] : m_widgets)
    for(auto widget : widgets)
      delete widget.widget;

  delete titleIcon;
  delete[] GuiEditor::g_currSaveFile;
  GuiEditor::g_currSaveFile = nullptr;
  GuiEditor::g_currSaveFileName = "";
  Widget::g_selectedCategory = "";

  backupNames.clear();
  saveFiles.clear();
}

void GuiEditor::draw() {
  Gui::beginDraw();

  std::stringstream ss;
  ss << "0x" << std::setfill('0') << std::setw(16) << std::uppercase << std::hex << Title::g_currTitle->getTitleID();

  Gui::drawRectangle(0, 0, Gui::g_framebuffer_width, Gui::g_framebuffer_height, currTheme.backgroundColor);
  Gui::drawRectangle(0, 0, Gui::g_framebuffer_width, 128, dominantColor);
  Gui::drawImage(0, 0, 128, 128, titleIcon, IMAGE_MODE_RGB24);
  Gui::drawImage(Gui::g_framebuffer_width - 128, 0, 128, 128, Account::g_currAccount->getProfileImage(), IMAGE_MODE_RGB24);
  Gui::drawShadow(0, 0, Gui::g_framebuffer_width, 128);


  Gui::drawTextAligned(font24, (Gui::g_framebuffer_width / 2), 10, textColor, Title::g_currTitle->getTitleName().c_str(), ALIGNED_CENTER);
  Gui::drawTextAligned(font20, (Gui::g_framebuffer_width / 2), 45, textColor, Title::g_currTitle->getTitleAuthor().c_str(), ALIGNED_CENTER);
  Gui::drawTextAligned(font20, (Gui::g_framebuffer_width / 2), 80, textColor, ss.str().c_str(), ALIGNED_CENTER);

  Widget::drawWidgets(this, m_widgets, 150, widgetPage * WIDGETS_PER_PAGE, (widgetPage + 1) * WIDGETS_PER_PAGE);

  Gui::drawRectangle((u32)((Gui::g_framebuffer_width - 1220) / 2), Gui::g_framebuffer_height - 73, 1220, 1, currTheme.textColor);

  if (GuiEditor::g_currSaveFile == nullptr) {
    Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 100, Gui::g_framebuffer_height - 50, currTheme.textColor, "\x03 - Backup     \x04 - Restore     \x02 - Back", ALIGNED_RIGHT);
    Gui::drawTextAligned(font24, (Gui::g_framebuffer_width / 2), (Gui::g_framebuffer_height / 2), currTheme.textColor, hasConfigFile ? "No save file loaded. Press \x08 to select one." : "No editor JSON file found. Editing is disabled.", ALIGNED_CENTER);
  } else
    Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 100, Gui::g_framebuffer_height - 50, currTheme.textColor, "\x03 - Apply changes     \x02 - Cancel     \x01 - OK", ALIGNED_RIGHT);

  if (m_widgets[Widget::g_selectedCategory].size() > WIDGETS_PER_PAGE) {
    for (u8 page = 0; page < widgetPageCnt[Widget::g_selectedCategory]; page++) {
      Gui::drawRectangle((Gui::g_framebuffer_width / 2) - widgetPageCnt[Widget::g_selectedCategory] * 15 + page * 30 , 615, 20, 20, currTheme.separatorColor);
      if (page == widgetPage)
        Gui::drawRectangled((Gui::g_framebuffer_width / 2) - widgetPageCnt[Widget::g_selectedCategory] * 15 + page * 30 + 4, 619, 12, 12, currTheme.highlightColor);
    }

    Gui::drawTextAligned(font24, (Gui::g_framebuffer_width / 2) - widgetPageCnt[Widget::g_selectedCategory] * 15 - 30, 598, currTheme.textColor, "\x05", ALIGNED_CENTER);
    Gui::drawTextAligned(font24, (Gui::g_framebuffer_width / 2) + widgetPageCnt[Widget::g_selectedCategory] * 15 + 18, 598, currTheme.textColor, "\x06", ALIGNED_CENTER);

  }

  Gui::endDraw();
}

bool GuiEditor::loadConfigFile(json &j) {
  std::stringstream path;
  path << CONFIG_ROOT << std::setfill('0') << std::setw(sizeof(u64) * 2) << std::uppercase << std::hex << Title::g_currTitle->getTitleID() << ".json";

  std::ifstream file(path.str().c_str());

  m_widgets.clear();

  if (file.fail())
    return false;

  try {
    file >> j;
  } catch (json::parse_error& e) {
		printf("Failed to parse JSON file.\n");
		return false;
	}

  return true;
}

void GuiEditor::createWidgets() {
  std::set<std::string> tempCategories;

  Widget::g_selectedRow = CATEGORIES;

  for (auto item : m_offsetFile["items"]) {
    if (item["name"] == nullptr || item["category"] == nullptr || item["intArgs"] == nullptr || item["strArgs"] == nullptr) continue;

    auto itemWidget = item["widget"];

    if (itemWidget == nullptr) continue;
    if (itemWidget["type"] == nullptr) continue;

    if (itemWidget["type"] == "int") {
      if (itemWidget["minValue"] == nullptr || itemWidget["maxValue"] == nullptr) continue;
      if (itemWidget["minValue"] >= itemWidget["maxValue"]) continue;

      m_widgets[item["category"]].push_back({ item["name"], new WidgetValue(&luaParser, itemWidget["minValue"], itemWidget["maxValue"], itemWidget.find("stepSize") != itemWidget.end() ? itemWidget["stepSize"].get<u32>() : 0) });
    }
    else if (itemWidget["type"] == "bool") {
      if (itemWidget["onValue"] == nullptr || itemWidget["offValue"] == nullptr) continue;
      if (itemWidget["onValue"] == itemWidget["offValue"]) continue;

      if(itemWidget["onValue"].is_number() && itemWidget["offValue"].is_number())
        m_widgets[item["category"]].push_back({ item["name"], new WidgetSwitch(&luaParser, itemWidget["onValue"].get<s32>(), itemWidget["offValue"].get<s32>()) });
      else if(itemWidget["onValue"].is_string() && itemWidget["offValue"].is_string())
        m_widgets[item["category"]].push_back({ item["name"], new WidgetSwitch(&luaParser, itemWidget["onValue"].get<std::string>(), itemWidget["offValue"].get<std::string>()) });
    }
    else if (itemWidget["type"] == "list") {
      if (itemWidget["listItemNames"] == nullptr || itemWidget["listItemValues"] == nullptr) continue;

      if (itemWidget["listItemValues"][0].is_number())
        m_widgets[item["category"]].push_back({ item["name"], new WidgetList(&luaParser, itemWidget["listItemNames"], itemWidget["listItemValues"].get<std::vector<s32>>()) });
      else if (itemWidget["listItemValues"][0].is_string())
        m_widgets[item["category"]].push_back({ item["name"], new WidgetList(&luaParser, itemWidget["listItemNames"], itemWidget["listItemValues"].get<std::vector<std::string>>()) });
    }

    m_widgets[item["category"]].back().widget->setLuaArgs(item["intArgs"], item["strArgs"]);

    tempCategories.insert(item["category"].get<std::string>());

  }

  Widget::g_categories.clear();
  std::copy(tempCategories.begin(), tempCategories.end(), std::back_inserter(Widget::g_categories));

  Widget::g_selectedCategory = Widget::g_categories[0];

  for (auto category : tempCategories)
    widgetPageCnt[category] = ceil(m_widgets[category].size() / WIDGETS_PER_PAGE);
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
if (GuiEditor::g_currSaveFile == nullptr) { /* No savefile loaded */

  if (kdown & KEY_MINUS) {
    if (!hasConfigFile) return;
    saveFiles.clear();

    if (m_offsetFile == nullptr) return;
    if (m_offsetFile["saveFilePaths"] == nullptr || m_offsetFile["files"] == nullptr || m_offsetFile["filetype"] == nullptr || m_offsetFile["items"] == nullptr) return;

    for (auto saveFilePath : m_offsetFile["saveFilePaths"])
      updateSaveFileList(saveFilePath.get<std::string>().c_str());

    (new ListSelector("Edit save file", "\x01 - Select      \x02 - Back", saveFiles))->setInputAction([&](u32 k, u16 selectedItem){
      if (k & KEY_A) {
        if (saveFiles.size() != 0) {
          size_t length;

          Widget::g_selectedWidgetIndex = 0;
          Widget::g_selectedCategory = "";
          Widget::g_selectedRow = CATEGORIES;

          GuiEditor::g_currSaveFileName = saveFiles[Gui::Gui::g_currListSelector->selectedItem].c_str();

          if (loadSaveFile(&GuiEditor::g_currSaveFile, &length, Title::g_currTitle->getTitleID(), Account::g_currAccount->getUserID(), GuiEditor::g_currSaveFileName.c_str()) == 0) {
              luaParser.setLuaSaveFileBuffer(g_currSaveFile, length);
              createWidgets();
              luaParser.luaInit(m_offsetFile["filetype"]);

              if (m_offsetFile["titleVersion"] != nullptr) {
                if (Title::g_currTitle->getTitleVersion() != m_offsetFile["titleVersion"]) {
                  std::string message = "The config file you're using was made for\nversion ";
                  message += m_offsetFile["titleVersion"];
                  message += " but your current version is ";
                  message += Title::g_currTitle->getTitleVersion();
                  message += ".\nAre you sure you want to continue?";

                (new MessageBox(message, YES_NO))->setSelectionAction([&](s8 selection) {
                    if (!selection) {
                      luaParser.luaDeinit();

                      delete[] GuiEditor::g_currSaveFile;
                      GuiEditor::g_currSaveFileName = "";
                      GuiEditor::g_currSaveFile = nullptr;

                      for (auto const& [category, widgets] : m_widgets)
                        for(auto widget : widgets)
                          delete widget.widget;

                      m_widgets.clear();
                      Widget::g_categories.clear();
                    }
                  })->show();
                }
              }
            }
            else {
              (new Snackbar("Failed to load save file! Is it empty?"))->show();
              delete[] GuiEditor::g_currSaveFile;
              GuiEditor::g_currSaveFile = nullptr;
              GuiEditor::g_currSaveFileName = "";

              for (auto const& [category, widgets] : m_widgets)
                for(auto widget : widgets)
                  delete widget.widget;

              m_widgets.clear();
            }
            Gui::Gui::g_currListSelector->hide();
          }
        }
      })->show();
    }

    if (kdown & KEY_B) {
      Gui::g_nextGui = GUI_MAIN;
    }

    if (kdown & KEY_X) {
      s16 res;

      if(!(res = backupSave(Title::g_currTitle->getTitleID(), Account::g_currAccount->getUserID())))
        (new Snackbar("Sucessfully created backup!"))->show();
      else (new Snackbar("An error occured while creating the backup! Error " + std::to_string(res)))->show();
    }

    if (kdown & KEY_Y) {
      updateBackupList();

      (new ListSelector("Restore Backup", "\x01 - Restore     \x03 - Delete      \x02 - Back", backupNames))->setInputAction([&](u32 k, u16 selectedItem){
        if (k & KEY_A) {
          if (backupNames.size() != 0) {
              (new MessageBox("Are you sure you want to inject this backup?", YES_NO))->setSelectionAction([&](s8 selection) {
                if (selection) {
                  s16 res;

                  if(!(res = restoreSave(Title::g_currTitle->getTitleID(), Account::g_currAccount->getUserID(), backupNames[Gui::Gui::g_currListSelector->selectedItem].c_str())))
                    (new Snackbar("Sucessfully restored backup!"))->show();
                  else (new Snackbar("An error occured while restoring the backup! Error " + std::to_string(res)))->show();

                  Gui::Gui::g_currListSelector->hide();
                }
              })->show();
          }
        }

        if (k & KEY_X) {
          std::stringstream path;
          path << "/EdiZon/" << std::setfill('0') << std::setw(16) << std::uppercase << std::hex << Title::g_currTitle->getTitleID();
          path << "/" << backupNames[Gui::Gui::g_currListSelector->selectedItem];
          deleteDirRecursively(path.str().c_str(), false);
          updateBackupList();

          if (Gui::Gui::g_currListSelector->selectedItem == backupNames.size() && Gui::Gui::g_currListSelector->selectedItem > 0)
            Gui::Gui::g_currListSelector->selectedItem--;
        }
      })->show();
    }

    if (kdown & KEY_ZL) {
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

    if (kdown & KEY_ZR) {
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
  } /* Savefile loaded */
  else {
    if (Widget::g_selectedRow == WIDGETS) { /* Widgets row */
      if (kdown & KEY_L) {
        if (widgetPage > 0)
          widgetPage--;
        Widget::g_selectedWidgetIndex = WIDGETS_PER_PAGE * widgetPage;
      }

      if (kdown & KEY_R) {
        if (widgetPage < widgetPageCnt[Widget::g_selectedCategory] - 1)
          widgetPage++;
        Widget::g_selectedWidgetIndex = WIDGETS_PER_PAGE * widgetPage ;
      }

      if (kdown & KEY_B) {
        Widget::g_selectedRow = CATEGORIES;
        Widget::g_selectedWidgetIndex = std::distance(Widget::g_categories.begin(), std::find(Widget::g_categories.begin(), Widget::g_categories.end(), Widget::g_selectedCategory));
      }

      if (kdown & KEY_UP) {
        if (Widget::g_selectedWidgetIndex > 0)
          Widget::g_selectedWidgetIndex--;
        widgetPage = floor(Widget::g_selectedWidgetIndex / WIDGETS_PER_PAGE);
      }

      if (kdown & KEY_DOWN) {
        if (Widget::g_selectedWidgetIndex < m_widgets[Widget::g_selectedCategory].size() - 1)
          Widget::g_selectedWidgetIndex++;
        widgetPage = floor(Widget::g_selectedWidgetIndex / WIDGETS_PER_PAGE);
      }

    } else { /* Categories row */
      if (kdown & KEY_B) {
        (new MessageBox("Are you sure you want to discard your changes?", YES_NO))->setSelectionAction([&](s8 selection) {
          if (selection) {
            luaParser.luaDeinit();

            delete[] GuiEditor::g_currSaveFile;
            GuiEditor::g_currSaveFileName = "";
            GuiEditor::g_currSaveFile = nullptr;

            for (auto const& [category, widgets] : m_widgets)
              for(auto widget : widgets)
                delete widget.widget;

            m_widgets.clear();
            Widget::g_categories.clear();
          }
        })->show();

        return;
      }

      if (kdown & KEY_UP) {
        if (Widget::g_selectedWidgetIndex > 0)
          Widget::g_selectedWidgetIndex--;
        Widget::g_selectedCategory = Widget::g_categories[Widget::g_selectedWidgetIndex];
      }

      if (kdown & KEY_DOWN) {
        if (Widget::g_selectedWidgetIndex < Widget::g_categories.size() - 1)
          Widget::g_selectedWidgetIndex++;
        Widget::g_selectedCategory = Widget::g_categories[Widget::g_selectedWidgetIndex];
      }
    }
    /* Categories and widgets row */
    if (kdown & KEY_X) {
      (new MessageBox("Are you sure you want to edit these values?", YES_NO))->setSelectionAction([&](s8 selection) {
        if (selection) {
          size_t size = 0;
          std::vector<u8> buffer;

          luaParser.getModifiedSaveFile(buffer, &size);

          if(!storeSaveFile(&buffer[0], size, Title::g_currTitle->getTitleID(), Account::g_currAccount->getUserID(), GuiEditor::g_currSaveFileName.c_str()))
            (new Snackbar("Sucessfully injected modified values!"))->show();
          else
            (new Snackbar("Injection of modified values failed!"))->show();

          delete[] GuiEditor::g_currSaveFile;
          GuiEditor::g_currSaveFile = nullptr;
          GuiEditor::g_currSaveFileName = "";

          for (auto const& [category, widgets] : m_widgets)
            for(auto widget : widgets)
              delete widget.widget;

          Widget::g_categories.clear();
          m_widgets.clear();
        }
      })->show();

      return;
    }

    Widget::handleInput(kdown, m_widgets);
  }
}

void GuiEditor::onTouch(touchPosition &touch) {
  //s8 widgetTouchPos = floor((touch.py - 150) / (static_cast<float>(WIDGET_HEIGHT) + WIDGET_SEPARATOR)) + WIDGETS_PER_PAGE * widgetPage;

  if (GuiEditor::g_currSaveFile == nullptr) {
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

    if (touch.px > Gui::g_framebuffer_width - 128 && touch.py < 128) {
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
  }
}

void GuiEditor::onGesture(touchPosition &startPosition, touchPosition &endPosition) {

}
