#include <switch.h>
#include <stdio.h>
#include <vector>
#include <algorithm>
#include <chrono>

#include <iostream>
#include <unistd.h>
#include <fcntl.h>

#include "gui.hpp"
#include "gui_main.hpp"
#include "gui_editor.hpp"

#include "update_manager.hpp"

#include "title.hpp"

#include "threads.hpp"

extern "C" {
  #include "theme.h"
}

#define NXLINK

#define LONG_PRESS_DELAY              70
#define LONG_PRESS_ACTIVATION_DELAY   300

bool updateThreadRunning = false;

Mutex mutexCurrGui;

size_t __nx_heap_size = 0x200000 * 32;

char* g_edizonPath;

Gui* currGui = nullptr;

void initTitles() {
  std::vector<FsSaveDataInfo> saveInfoList;
  _getSaveList(saveInfoList);

  for (auto saveInfo : saveInfoList) {

    if (Title::g_titles.find(saveInfo.titleID) == Title::g_titles.end())
      Title::g_titles.insert({(u64)saveInfo.titleID, new Title(saveInfo)});

    Title::g_titles[saveInfo.titleID]->addUserID(saveInfo.userID);

    if (Account::g_accounts.find(saveInfo.userID) == Account::g_accounts.end())
      Account::g_accounts.insert({ static_cast<u128>(saveInfo.userID), new Account(saveInfo.userID) });
  }
}

void update(void *args) {
  auto begin = std::chrono::steady_clock::now();
  u32 inputTicker = 0;
  u8 touchCntOld = hidTouchCount(), touchCnt;
  u32 kheld = 0, kheldOld = 0;
  u32 kdown = 0;
  touchPosition touch;
  touchPosition touchEnd;

  while (updateThreadRunning) {
    hidScanInput();
    kdown = hidKeysDown(CONTROLLER_P1_AUTO);
    kheld = hidKeysHeld(CONTROLLER_P1_AUTO);

    begin = std::chrono::steady_clock::now();

    mutexLock(&mutexCurrGui);

    if (currGui != nullptr) {
      if (kdown || hidKeysUp(CONTROLLER_P1_AUTO)) {
        if (Gui::g_currMessageBox != nullptr)
          Gui::g_currMessageBox->onInput(kdown);
        else if (Gui::g_currListSelector != nullptr)
          Gui::g_currListSelector->onInput(kdown);
        else if (Gui::g_currKeyboard != nullptr)
          Gui::g_currKeyboard->onInput(kdown);
        else
          currGui->onInput(kdown);
      }

      if (kheld & (KEY_LEFT | KEY_RIGHT | KEY_UP | KEY_DOWN)) inputTicker++;
      else inputTicker = 0;

      if (kheld != kheldOld)
        inputTicker = 0;

      if (inputTicker > LONG_PRESS_ACTIVATION_DELAY && (inputTicker % LONG_PRESS_DELAY) == 0) {
        if (Gui::g_currMessageBox != nullptr)
          Gui::g_currMessageBox->onInput(kheld);
        else if (Gui::g_currListSelector != nullptr)
          Gui::g_currListSelector->onInput(kheld);
        else if (Gui::g_currKeyboard != nullptr)
          Gui::g_currKeyboard->onInput(kheld);
        else
          currGui->onInput(kheld);
      }

      currGui->update();
    }

    touchCnt = hidTouchCount();

    if (touchCnt > touchCntOld)
      hidTouchRead(&touch, 0);

    if (touchCnt < touchCntOld) {
      if (Gui::g_currMessageBox != nullptr)
        Gui::g_currMessageBox->onTouch(touch);
      else if (Gui::g_currListSelector != nullptr)
        Gui::g_currListSelector->onTouch(touch);
      else {
        currGui->onTouch(touchEnd);
        currGui->onGesture(touch, touchEnd);
      }
    }

    hidTouchRead(&touchEnd, 0);

    touchCntOld = touchCnt;
    kheldOld = kheld;

    if (kdown & KEY_PLUS) {
      delete currGui;
    }

    mutexUnlock(&mutexCurrGui);

    svcSleepThread(1.0E6 - std::chrono::duration<double, std::nano>(std::chrono::steady_clock::now() - begin).count());
  }
}

int main(int argc, char** argv) {
  socketInitializeDefault();

#ifdef NXLINK
  nxlinkStdio();
#endif

  int file = open("/EdiZon/EdiZon.log", O_APPEND | O_WRONLY);

  if (file >= 0) {
    fflush(stdout);
    dup2(file, STDOUT_FILENO);
    fflush(stderr);
    dup2(file, STDERR_FILENO);
  }

  gfxInitDefault();
  setsysInitialize();
  ColorSetId colorSetId;
  setsysGetColorSetId(&colorSetId);
  setTheme(colorSetId);

  initTitles();

  Gui::g_nextGui = GUI_MAIN;

  g_edizonPath = new char[strlen(argv[0])];
  strcpy(g_edizonPath, argv[0] + 5);

  mutexInit(&mutexCurrGui);

  updateThreadRunning = true;
  Threads::create(&update);

  while (appletMainLoop()) {
    if (Gui::g_nextGui != GUI_INVALID) {
      mutexLock(&mutexCurrGui);
      if (currGui != nullptr)
        delete currGui;

      switch (Gui::g_nextGui) {
        case GUI_MAIN:
          currGui = new GuiMain();
          break;
        case GUI_EDITOR:
          currGui = new GuiEditor();
          break;
        default: break;
      }
      Gui::g_nextGui = GUI_INVALID;
      mutexUnlock(&mutexCurrGui);
    } else if (currGui == nullptr) break;

    currGui->draw();

    if (GuiMain::g_shouldUpdate) {
      Gui::g_currMessageBox->hide();

      UpdateManager updateManager;

      switch (updateManager.checkUpdate()) {
        case NONE: (new MessageBox("Latest configs and scripts are already installed!", MessageBox::OKAY))->show(); break;
        case ERROR: (new MessageBox("An error while downloading the updates has occured.", MessageBox::OKAY))->show(); break;
        case EDITOR: (new MessageBox("Updated editor configs and scripts to the latest version!", MessageBox::OKAY))->show(); break;
        case EDIZON: (new MessageBox("Updated EdiZon and editor configs and scripts to\nthe latest version! Please restart EdiZon!", MessageBox::OKAY))->show(); break;
      }

      GuiMain::g_shouldUpdate = false;
    }
  }


  updateThreadRunning = false;

  Threads::joinAll();

  for (auto it = Title::g_titles.begin(); it != Title::g_titles.end(); it++)
    delete it->second;

  for (auto it = Account::g_accounts.begin(); it != Account::g_accounts.end(); it++)
    delete it->second;

  Title::g_titles.clear();
  Account::g_accounts.clear();

  close(file);

  socketExit();

  gfxExit();

  return 0;
}
