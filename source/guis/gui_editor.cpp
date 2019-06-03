#include "guis/gui_editor.hpp"

#include "helpers/title.hpp"
#include "helpers/save.hpp"

#include "helpers/editor_config_parser.hpp"
#include "scripting/interpreter.hpp"
#include "scripting/lua_interpreter.hpp"
#include "scripting/python_interpreter.hpp"

#include "upload_manager.hpp"

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

GuiEditor::GuiEditor() : Gui() {
  m_titleIcon.reserve(128 * 128 * 3);
  std::vector<u8> smallTitleIcon(32 * 32 * 3);
  std::map<u32, u16> colors;

  m_dominantColor = Gui::makeColor(0xA0, 0xA0, 0xA0, 0xFF);

  if (Title::g_currTitle->getTitleID() == Title::g_activeTitle) {
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


  Gui::resizeImage(Title::g_currTitle->getTitleIcon(), &m_titleIcon[0], 256, 256, 128, 128);
  Gui::resizeImage(Title::g_currTitle->getTitleIcon(), &smallTitleIcon[0], 256, 256, 32, 32);

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
      m_dominantColor = colorCandidate;
    }
  }

  m_textColor = (m_dominantColor.r > 0x80 && m_dominantColor.g > 0x80 && m_dominantColor.b > 0x80) ? COLOR_BLACK : COLOR_WHITE;

  Widget::g_widgetPage = 0;
  Widget::g_selectedWidgetIndex = 0;
  Widget::g_selectedCategory = "";

  std::stringstream path;
  path << CONFIG_ROOT << std::setfill('0') << std::setw(sizeof(u64) * 2) << std::uppercase << std::hex << Title::g_currTitle->getTitleID() << ".json";
  m_configFileResult = EditorConfigParser::loadConfigFile(Title::g_currTitle->getTitleID(), path.str(), &m_interpreter);
}

GuiEditor::~GuiEditor() {
  if (GuiEditor::g_currSaveFileName != "") {
    for (auto const& [category, widgets] : m_widgets)
      for(auto widget : widgets)
        delete widget.widget;

    delete m_interpreter;
  }

  GuiEditor::g_currSaveFile.clear();
  GuiEditor::g_currSaveFileName = "";
  EditorConfigParser::g_currConfigAuthor = "";
  Widget::g_selectedCategory = "";

  m_backupTitles.clear();
  m_backupPaths.clear();
  m_backupFolderNames.clear();

  m_saveFiles.clear();
}

void GuiEditor::update() {
  Gui::update();
}

void GuiEditor::draw() {
  Gui::beginDraw();

  std::stringstream ssTitleId;
  ssTitleId << std::setfill('0') << std::setw(16) << std::uppercase << std::hex << Title::g_currTitle->getTitleID();

  Gui::drawRectangle(0, 0, Gui::g_framebuffer_width, Gui::g_framebuffer_height, currTheme.backgroundColor);

  Widget::drawWidgets(this, m_widgets, Widget::g_widgetPage * WIDGETS_PER_PAGE, (Widget::g_widgetPage + 1) * WIDGETS_PER_PAGE);

  Gui::drawRectangle(0, 0, Gui::g_framebuffer_width, 128, m_dominantColor);
  Gui::drawImage(0, 0, 128, 128, &m_titleIcon[0], IMAGE_MODE_RGB24);
  Gui::drawImage(Gui::g_framebuffer_width - 128, 0, 128, 128, Account::g_currAccount->getProfileImage(), IMAGE_MODE_RGB24);
  Gui::drawShadow(0, 0, Gui::g_framebuffer_width, 128);

  Gui::drawTextAligned(font24, (Gui::g_framebuffer_width / 2), 10, m_textColor, Title::g_currTitle->getTitleName().c_str(), ALIGNED_CENTER);

  Gui::drawRectangle(0, Gui::g_framebuffer_height - 73, Gui::g_framebuffer_width, 73, currTheme.backgroundColor);
  Gui::drawRectangle((u32)((Gui::g_framebuffer_width - 1220) / 2), Gui::g_framebuffer_height - 73, 1220, 1, currTheme.textColor);

  if (GuiEditor::g_currSaveFileName == "") {
    Gui::drawTextAligned(font20, (Gui::g_framebuffer_width / 2), 50, m_textColor, Title::g_currTitle->getTitleAuthor().c_str(), ALIGNED_CENTER);
    Gui::drawTextAligned(font20, (Gui::g_framebuffer_width / 2), 80, m_textColor, ssTitleId.str().c_str(), ALIGNED_CENTER);

    Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 50, Gui::g_framebuffer_height - 50, currTheme.textColor, "\uE0E6 Next game     \uE0E7 Next user     \uE0E2 Backup     \uE0E3 Restore     \uE0E1 Back", ALIGNED_RIGHT);
    switch (m_configFileResult) {
      case 0:
        Gui::drawTextAligned(font24, (Gui::g_framebuffer_width / 2), (Gui::g_framebuffer_height / 2), currTheme.textColor, "Press \uE0F0 to load and edit your save file.", ALIGNED_CENTER);
        break;
      case 1:
        Gui::drawTextAligned(font24, (Gui::g_framebuffer_width / 2), (Gui::g_framebuffer_height / 2), currTheme.textColor, "No config for this game was found. Editing is disabled.", ALIGNED_CENTER);
        break;
      case 2:
        Gui::drawTextAligned(font24, (Gui::g_framebuffer_width / 2), (Gui::g_framebuffer_height / 2), currTheme.textColor, "Syntax error in config file! Editing is disabled.", ALIGNED_CENTER);
        break;
      case 3:
        Gui::drawTextAligned(font24, (Gui::g_framebuffer_width / 2), (Gui::g_framebuffer_height / 2), currTheme.textColor, "The config isn't compatible with your game's version. Editing is disabled.", ALIGNED_CENTER);
        break;
      case 4:
        Gui::drawTextAligned(font24, (Gui::g_framebuffer_width / 2), (Gui::g_framebuffer_height / 2), currTheme.textColor, "The config redirected more than 5 times. Editing is disabled.", ALIGNED_CENTER);
        break;
    }

  } else {
    std::stringstream ssMultiplier;
    ssMultiplier << "\uE074 : x";
    ssMultiplier << Widget::g_stepSizeMultiplier;

    if (EditorConfigParser::g_currConfigAuthor != "")
      Gui::drawTextAligned(font20, (Gui::g_framebuffer_width / 2), 80, m_textColor, std::string("Config created by " + EditorConfigParser::g_currConfigAuthor).c_str(), ALIGNED_CENTER);
    
    Gui::drawTextAligned(font20, 50, Gui::g_framebuffer_height - 50, currTheme.textColor, ssMultiplier.str().c_str(), ALIGNED_LEFT);
    Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 50, Gui::g_framebuffer_height - 50, currTheme.textColor, "\uE105 Increase multiplier     \uE0E2 Apply changes     \uE0E1 Cancel     \uE0E0 OK", ALIGNED_RIGHT);
  }

  if (m_widgets[Widget::g_selectedCategory].size() > WIDGETS_PER_PAGE) {
    for (u8 page = 0; page < Widget::g_widgetPageCnt[Widget::g_selectedCategory]; page++) {
      Gui::drawRectangle((Gui::g_framebuffer_width / 2) - Widget::g_widgetPageCnt[Widget::g_selectedCategory] * 15 + page * 30 , 608, 20, 20, currTheme.separatorColor);
      
      if (page == Widget::g_widgetPage)
        Gui::drawRectangled((Gui::g_framebuffer_width / 2) - Widget::g_widgetPageCnt[Widget::g_selectedCategory] * 15 + page * 30 + 4, 612, 12, 12, currTheme.highlightColor);
    }

    Gui::drawTextAligned(font20, (Gui::g_framebuffer_width / 2) - Widget::g_widgetPageCnt[Widget::g_selectedCategory] * 15 - 30, 602, currTheme.textColor, "\uE0A4", ALIGNED_CENTER);
    Gui::drawTextAligned(font20, (Gui::g_framebuffer_width / 2) + Widget::g_widgetPageCnt[Widget::g_selectedCategory] * 15 + 23, 602, currTheme.textColor, "\uE0A5", ALIGNED_CENTER);

  }

  Gui::endDraw();
}

void GuiEditor::updateBackupList() {
  DIR *dir_batch;
  DIR *dir_users;
  DIR *dir_titles;

  struct dirent *ent_timestamp;
  struct dirent *ent_user;

  std::vector<std::string> backups;

  std::string metadataUsername;

  m_backupTitles.clear();
  m_backupPaths.clear();
  m_backupFolderNames.clear();

  std::stringstream path;

  path << EDIZON_DIR "/restore";
  if ((dir_titles = opendir(path.str().c_str())) != nullptr) {
    while ((ent_timestamp = readdir(dir_titles)) != nullptr) {
      if (ent_timestamp->d_type != DT_DIR) continue;

      metadataUsername = GuiEditor::readMetaDataUsername(path.str() + "/" + std::string(ent_timestamp->d_name) + "/edizon_save_metadata.json");
      if (metadataUsername.empty())
        metadataUsername = "By an unknown user [/restore]";
      else
        metadataUsername = "By " + metadataUsername + " [/restore]";

      backups.push_back(std::string(ent_timestamp->d_name) +  ", " + metadataUsername);
      backups.push_back(path.str() + "/" + std::string(ent_timestamp->d_name));
      backups.push_back(std::string(ent_timestamp->d_name));
    }
    closedir(dir_titles);
  }

  //Read batch saves
  if ((dir_batch = opendir(EDIZON_DIR "/batch_saves")) != nullptr) {
    while ((ent_timestamp = readdir(dir_batch)) != nullptr) {
      path.str("");
      path << EDIZON_DIR "/batch_saves/" << std::string(ent_timestamp->d_name);
      if ((dir_users = opendir(path.str().c_str())) != nullptr) {
        while ((ent_user = readdir(dir_users)) != nullptr) {
          if (ent_user->d_type != DT_DIR) continue;

          path.str("");
          path << EDIZON_DIR "/batch_saves/" << std::string(ent_timestamp->d_name) << "/" << std::string(ent_user->d_name) << "/" << std::setfill('0') << std::setw(16) << std::uppercase << std::hex << Title::g_currTitle->getTitleID();
          if ((dir_titles = opendir(path.str().c_str())) != nullptr) {
            metadataUsername = GuiEditor::readMetaDataUsername(path.str() + "/edizon_save_metadata.json");

            if (metadataUsername.empty())
              metadataUsername = "By an unknown user [Batch]";
            else
              metadataUsername = "By " + metadataUsername + " [Batch]";

            backups.push_back(std::string(ent_timestamp->d_name) +  ", " + metadataUsername);
            backups.push_back(path.str());
            backups.push_back(std::string(ent_timestamp->d_name));

            closedir(dir_titles);
          }
        }
        closedir(dir_users);
      }
    }
    closedir(dir_batch);
  }

    path.str("");

  //Read root saves
  path << EDIZON_DIR "/saves/" << std::setfill('0') << std::setw(16) << std::uppercase << std::hex << Title::g_currTitle->getTitleID();
  if ((dir_titles = opendir(path.str().c_str())) != nullptr) {
    while ((ent_timestamp = readdir(dir_titles)) != nullptr) {
      if (ent_timestamp->d_type != DT_DIR) continue;

      metadataUsername = GuiEditor::readMetaDataUsername(path.str() + "/" + std::string(ent_timestamp->d_name) + "/edizon_save_metadata.json");
      if (metadataUsername.empty())
        metadataUsername = "By an unknown user";
      else
        metadataUsername = "By " + metadataUsername;

      backups.push_back(std::string(ent_timestamp->d_name) +  ", " + metadataUsername);
      backups.push_back(path.str() + "/" + std::string(ent_timestamp->d_name));
      backups.push_back(std::string(ent_timestamp->d_name));
    }
    closedir(dir_titles);
  }

  
  for (auto iter = backups.begin(); iter != backups.end();) {
    m_backupTitles.push_back(*iter++);
    m_backupPaths.push_back(*iter++);
    m_backupFolderNames.push_back(*iter++);
  }

  std::reverse(m_backupTitles.begin(), m_backupTitles.end());
  std::reverse(m_backupPaths.begin(), m_backupPaths.end());
  std::reverse(m_backupFolderNames.begin(), m_backupFolderNames.end());

}

std::string GuiEditor::readMetaDataUsername(std::string path) {
  json metadata_json;

  std::ifstream metadata_file (path);

  if (metadata_file.is_open()) {
    metadata_file >> metadata_json;
    metadata_file.close();
    try {
      return metadata_json["user_name"].get<std::string>();
    } catch (json::parse_error& e) { }
  }

  return "";
}

void GuiEditor::updateSaveFileList(std::vector<std::string> saveFilePath, std::string files, u8 configIndex) {
  DIR *dir;
  struct dirent *ent;
  FsFileSystem fs;

  std::vector<std::string> pathsOld;
  std::vector<std::string> paths;

  if (mountSaveByTitleAccountIDs(Title::g_currTitle->getTitleID(), Account::g_currAccount->getUserID(), fs))
    return;

  if (saveFilePath[0] != "") {
    dir = opendir("save:/");

    while ((ent = readdir(dir)) != nullptr) {
      if (std::regex_match(std::string(ent->d_name), std::regex(saveFilePath[0]))) {
        pathsOld.push_back(std::string(ent->d_name) + "/");
      }
    }

    closedir(dir);

    for (u16 i = 1; i < saveFilePath.size(); i++) {
      for (auto path : pathsOld) {
        dir = opendir(path.c_str());
        while ((ent = readdir(dir)) != nullptr) {
          if (std::regex_match(std::string(ent->d_name), std::regex(saveFilePath[i]))) {
            std::string newPath = path;
            newPath += "/";
            newPath += ent->d_name;
            paths.push_back(newPath);
          }
        }

        closedir(dir);
      }

      pathsOld = paths;
      paths.clear();
    }
  } else pathsOld.push_back("");

  for (auto path : pathsOld) {
    std::string finalSaveFilePath = std::string("save:/") + path;
    if ((dir = opendir(finalSaveFilePath.c_str())) != nullptr) {
      std::regex validSaveFileNames(files);

      while ((ent = readdir(dir)) != nullptr) {
        if (std::regex_match(ent->d_name, validSaveFileNames))
          m_saveFiles.push_back({ path + ent->d_name, configIndex });
      }
      closedir(dir);
    }
  }

  fsdevUnmountDevice(SAVE_DEV);
  fsFsClose(&fs);

}

void uploadBackup(std::string path, std::string fileName) {
  UploadManager um;
  static std::stringstream hashStr;

  (new MessageBox("Uploading savefile...\n \nPress \uE0E1 to cancel.", MessageBox::NONE))->show();
  requestDraw();

  char serial[0x19];
  u8 serialHash[0x20];
  setsysGetSerialNumber(serial);


  Sha256Context shaCtx;
  sha256ContextCreate(&shaCtx);
  sha256ContextUpdate(&shaCtx, (u8 *)serial, 0x19);
  sha256ContextGetHash(&shaCtx, serialHash);

  hashStr.str("");

  for (u8 i = 0; i < 0x20; i++) {
    hashStr << std::hex << (serialHash[i] & 0x0F);
    hashStr << std::hex << (serialHash[i] >>   4);
  }

  static std::string retCode;
  retCode = um.upload(path, fileName, Title::g_currTitle, hashStr.str());
  retCode = retCode.substr(0, retCode.find("\n") - 1);

  if (retCode.length() == 6) {
    (new MessageBox("", MessageBox::OKAY))->setCustomDraw([&](Gui *gui, s16 x, s16 y){
      u32 w, h;
      gui->drawTextAligned(font20, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2 - 100, currTheme.textColor, "Upload finished!\n \n Visit edizon.werwolv.net and enter this code\n to get your link!", ALIGNED_CENTER);

      gui->getTextDimensions(font24, retCode.c_str(), &w, &h);

      gui->drawRectangle(x + (780 / 2) - (w / 2) - 20, y + 210, w + 40, h + 20, currTheme.tooltipColor);
      gui->drawTextAligned(font24, x + (780 / 2), y + 210, currTheme.textColor, retCode.c_str(), ALIGNED_CENTER);
    })->show();
  } 
  else
    (new MessageBox("Upload failed!\n \n" + retCode, MessageBox::OKAY))->show(); 
}

void GuiEditor::onInput(u32 kdown) {
if (GuiEditor::g_currSaveFileName == "") { /* No savefile loaded */

  if (kdown & KEY_MINUS) {
    if (m_configFileResult != 0) return;
    m_saveFiles.clear();

    for (u8 configIndex = 0; configIndex < EditorConfigParser::getConfigFile().size(); configIndex++)
      updateSaveFileList(EditorConfigParser::getConfigFile()[configIndex]["saveFilePaths"], EditorConfigParser::getConfigFile()[configIndex]["files"], configIndex);

    static std::vector<std::string> saveFileNames;

    saveFileNames.clear();

    for (auto saveFile : m_saveFiles)
      saveFileNames.push_back(saveFile.fileName);

    (new ListSelector("Edit save file", "\uE0E0  Select      \uE0E1  Back", saveFileNames))->setInputAction([&](u32 k, u16 selectedItem) {
      if (k & KEY_A) {
        if (m_saveFiles.size() != 0) {
          size_t length;
          Widget::g_selectedWidgetIndex = 0;
          Widget::g_selectedCategory = "";
          Widget::g_selectedRow = CATEGORIES;
          Widget::g_categoryYOffset = 0;
          
          (new MessageBox("Starting up editor...", MessageBox::NONE))->show();
          requestDraw();
          Gui::g_currMessageBox->hide();

          GuiEditor::g_currSaveFileName = m_saveFiles[Gui::g_currListSelector->selectedItem].fileName.c_str();

          if (loadSaveFile(&GuiEditor::g_currSaveFile, &length, Title::g_currTitle->getTitleID(), Account::g_currAccount->getUserID(), GuiEditor::g_currSaveFileName.c_str()) == 0) {
              m_interpreter->setSaveFileBuffer(&g_currSaveFile[0], length, EditorConfigParser::getOptionalValue<std::string>(EditorConfigParser::getConfigFile(), "encoding", "ascii"));
              EditorConfigParser::createWidgets(m_widgets, *m_interpreter, this->m_saveFiles[selectedItem].configIndex);

              if (EditorConfigParser::getConfigFile()[this->m_saveFiles[selectedItem].configIndex]["startupMessage"] != nullptr)
                (new MessageBox(EditorConfigParser::getConfigFile()[this->m_saveFiles[selectedItem].configIndex]["startupMessage"], MessageBox::OKAY))->show();

              if(!m_interpreter->initialize(EditorConfigParser::getConfigFile()[this->m_saveFiles[selectedItem].configIndex]["filetype"])) {
                m_interpreter->deinitialize();
                Gui::g_currMessageBox->hide();

                GuiEditor::g_currSaveFile.clear();
                GuiEditor::g_currSaveFileName = "";

                for (auto const& [category, widgets] : m_widgets)
                  for(auto widget : widgets)
                    delete widget.widget;

                m_widgets.clear();
                Widget::g_categories.clear();
                (new Snackbar("Failed to start script interpreter! Syntax error or script not found."))->show();

                Gui::g_currListSelector->hide();
                return;
              }
            }
            else {
              (new Snackbar("Failed to load save file! Is it empty?"))->show();
              GuiEditor::g_currSaveFile.clear();
              GuiEditor::g_currSaveFileName = "";

              for (auto const& [category, widgets] : m_widgets)
                for(auto widget : widgets)
                  delete widget.widget;

              m_widgets.clear();
            }
            Gui::g_currListSelector->hide();
          }
        }
      })->show();
    }

    if (kdown & KEY_B) {
      Gui::g_nextGui = GUI_MAIN;
    }

    if (kdown & KEY_X) {
      s16 res;

      time_t t = time(nullptr);
      static char backupName[65];
      std::stringstream initialText;
      initialText << std::put_time(std::gmtime(&t), "%Y%m%d_%H%M%S");
      if(!Gui::requestKeyboardInput("Backup name", "Please enter a name for the backup to be saved under.", initialText.str(), SwkbdType_QWERTY, backupName, 32))
        return;
      
      (new MessageBox("Extracting save file.\n \nThis may take a while...", MessageBox::NONE))->show();
      requestDraw();

      if(!(res = backupSave(Title::g_currTitle->getTitleID(), Account::g_currAccount->getUserID(), false, backupName))) {
        (new MessageBox("Successfully created backup!\n \n Would you like to upload it to anonfile.com?", MessageBox::YES_NO))->setSelectionAction([&](u8 selection) {
          if (selection) {
            std::stringstream backupPath;
            backupPath << EDIZON_DIR "/saves/" << std::uppercase << std::setfill('0') 
            << std::setw(sizeof(Title::g_currTitle->getTitleID())*2) 
            << std::hex << Title::g_currTitle->getTitleID()
            << "/" << std::string(backupName);

            uploadBackup(backupPath.str(), backupName);
          } else Gui::g_currMessageBox->hide();
        })->show();
        
      }
      else {
        Gui::g_currMessageBox->hide();
        switch(res) {
          case 1: (new Snackbar("Failed to mount save file!"))->show(); break;
          case 2: (new Snackbar("A backup with this name already exists!"))->show(); break;
          case 3: (new Snackbar("Failed to create backup!"))->show(); break;
        }
      }
    }

    if (kdown & KEY_Y) {
      updateBackupList();

      (new ListSelector("Restore Backup", "\uE0F0  Upload     \uE0E0  Restore     \uE0E2  Delete     \uE0E1  Back", m_backupTitles))->setInputAction([&](u32 k, u16 selectedItem){
        if (k & KEY_A) {
          if (m_backupTitles.size() != 0) {
              (new MessageBox("Are you sure you want to inject this backup?", MessageBox::YES_NO))->setSelectionAction([&](s8 selection) {
                if (selection) {
                  s16 res;

                  if(!(res = restoreSave(Title::g_currTitle->getTitleID(), Account::g_currAccount->getUserID(), m_backupPaths[Gui::g_currListSelector->selectedItem].c_str())))
                    (new Snackbar("Successfully restored backup!"))->show();
                  else (new Snackbar("An error occured while restoring the backup! Error " + std::to_string(res)))->show();

                  Gui::g_currListSelector->hide();
                  Gui::g_currMessageBox->hide();
                } else Gui::g_currMessageBox->hide();
              })->show();
          }
        }

        if (k & KEY_X) {
          std::stringstream path;
          deleteDirRecursively(m_backupPaths[Gui::g_currListSelector->selectedItem].c_str(), false);
          updateBackupList();

          if (Gui::g_currListSelector->selectedItem == m_backupTitles.size() && Gui::g_currListSelector->selectedItem > 0)
            Gui::g_currListSelector->selectedItem--;
        }

        if (k & KEY_MINUS) {
          uploadBackup(m_backupPaths[Gui::g_currListSelector->selectedItem], m_backupFolderNames[Gui::g_currListSelector->selectedItem]);

          Gui::g_currListSelector->hide();
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
        if (Widget::g_widgetPage > 0)
          Widget::g_widgetPage--;
        Widget::g_selectedWidgetIndex = WIDGETS_PER_PAGE * Widget::g_widgetPage;
      }

      if (kdown & KEY_R) {
        if (Widget::g_widgetPage < Widget::g_widgetPageCnt[Widget::g_selectedCategory] - 1)
          Widget::g_widgetPage++;
        Widget::g_selectedWidgetIndex = WIDGETS_PER_PAGE * Widget::g_widgetPage ;
      }

      if (kdown & KEY_B || kdown & KEY_LSTICK_LEFT || kdown & KEY_RSTICK_LEFT) {
        Widget::g_selectedRow = CATEGORIES;
        Widget::g_selectedWidgetIndex = std::distance(Widget::g_categories.begin(), std::find(Widget::g_categories.begin(), Widget::g_categories.end(), Widget::g_selectedCategory));
      }

      if (kdown & KEY_LSTICK_UP || kdown & KEY_RSTICK_UP) {
        if (Widget::g_selectedWidgetIndex > 0)
          Widget::g_selectedWidgetIndex--;
        Widget::g_widgetPage = floor(Widget::g_selectedWidgetIndex / WIDGETS_PER_PAGE);
      }

      if (kdown & KEY_LSTICK_DOWN || kdown & KEY_RSTICK_DOWN) {
        if (Widget::g_selectedWidgetIndex < m_widgets[Widget::g_selectedCategory].size() - 1)
          Widget::g_selectedWidgetIndex++;
        Widget::g_widgetPage = floor(Widget::g_selectedWidgetIndex / WIDGETS_PER_PAGE);
      }

    } else { /* Categories row */
      if (kdown & KEY_B) {
        (new MessageBox("Are you sure you want to discard your changes?", MessageBox::YES_NO))->setSelectionAction([&](s8 selection) {
          if (selection) {
            m_interpreter->deinitialize();

            GuiEditor::g_currSaveFile.clear();
            GuiEditor::g_currSaveFileName = "";

            for (auto const& [category, widgets] : m_widgets)
              for(auto widget : widgets)
                delete widget.widget;

            m_widgets.clear();
            Widget::g_categories.clear();
            Gui::g_currMessageBox->hide();
          } else Gui::g_currMessageBox->hide();
        })->show();

        return;
      }

      if (kdown & KEY_LSTICK_UP || kdown & KEY_RSTICK_UP) {
        if (Widget::g_selectedWidgetIndex > 0) {
          Widget::g_selectedWidgetIndex--;

          if (Widget::g_selectedWidgetIndex < Widget::g_categories.size() - 7 && Widget::g_categoryYOffset != 0)
            Widget::g_categoryYOffset--;
        }
        Widget::g_selectedCategory = Widget::g_categories[Widget::g_selectedWidgetIndex];
        Widget::g_widgetPage = 0;
      }

      if (kdown & KEY_LSTICK_DOWN || kdown & KEY_RSTICK_DOWN) {
        if (Widget::g_selectedWidgetIndex < Widget::g_categories.size() - 1) {
          Widget::g_selectedWidgetIndex++;

          if (Widget::g_selectedWidgetIndex > 6 && Widget::g_categoryYOffset < Widget::g_categories.size() - 8)
            Widget::g_categoryYOffset++;
        }
        Widget::g_selectedCategory = Widget::g_categories[Widget::g_selectedWidgetIndex];
        Widget::g_widgetPage = 0;
      }
    }
    /* Categories and widgets row */
    if (kdown & KEY_X) {
      (new MessageBox(EditorConfigParser::g_betaTitles[Title::g_currTitle->getTitleID()] ? "Do you want to apply these changes? \n Make sure you have a working backup of your \n save data as this config is still in beta!" :"Do you want to apply these changes?", MessageBox::YES_NO))->setSelectionAction([&](s8 selection) {
        if (selection) {
          std::vector<u8> buffer;

          m_interpreter->getModifiedSaveFile(buffer);

          if(buffer.empty())
            (new Snackbar("Injection of modified values failed!"))->show();
          else {
            if(!storeSaveFile(&buffer[0], buffer.size(), Title::g_currTitle->getTitleID(), Account::g_currAccount->getUserID(), GuiEditor::g_currSaveFileName.c_str()))
              (new Snackbar("Successfully injected modified values!"))->show();
            else
              (new Snackbar("Injection of modified values failed!"))->show();
          }

          m_interpreter->deinitialize();
          GuiEditor::g_currSaveFile.clear();
          GuiEditor::g_currSaveFileName = "";

          Widget::g_widgetPage = 0;

          for (auto const& [category, widgets] : m_widgets)
            for(auto widget : widgets)
              delete widget.widget;

          Widget::g_categories.clear();
          m_widgets.clear();
          Gui::g_currMessageBox->hide();
        } else Gui::g_currMessageBox->hide();
      })->show();

      return;
    }

    Widget::handleInput(kdown, m_widgets);
  }
}

void GuiEditor::onTouch(touchPosition &touch) {
  if (GuiEditor::g_currSaveFileName == "") {
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
  } else {
      //s8 widgetTouchPos = floor((touch.py - 150) / (static_cast<float>(WIDGET_HEIGHT) + WIDGET_SEPARATOR)) + WIDGETS_PER_PAGE * Widget::g_widgetPage;
    Widget::handleTouch(touch, m_widgets);
  }
}

void GuiEditor::onGesture(touchPosition startPosition, touchPosition endPosition, bool finish) {

}
