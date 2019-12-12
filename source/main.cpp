#include <edizon.h>
#include <stdio.h>
#include <vector>
#include <algorithm>
#include <chrono>
#include <thread>

#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <curl/curl.h>

#include "guis/gui.hpp"
#include "guis/gui_main.hpp"
#include "guis/gui_editor.hpp"
#include "guis/gui_tx_warning.hpp"
#include "guis/gui_cheats.hpp"
#include "guis/gui_about.hpp"
#include "guis/gui_guide.hpp"

#include "helpers/title.hpp"

#include "theme.h"
#include "helpers/util.h"
#include "helpers/config.hpp"

#define LONG_PRESS_DELAY              2
#define LONG_PRESS_ACTIVATION_DELAY   300

char* g_edizonPath;

static int debugOutputFile;

static bool updateThreadRunning = false;
static Mutex mutexCurrGui;
static Gui* currGui = nullptr;
static s64 inputTicker = 0;

static u32 kheld = 0, kheldOld = 0;
static u32 kdown = 0;

void initTitles() {
  std::vector<FsSaveDataInfo> saveInfoList;

  _getSaveList(saveInfoList);

  s32 userCount = 0;
  s32 foundUserCount = 0;

  accountGetUserCount(&userCount);

  AccountUid userIDs[userCount];
  accountListAllUsers(userIDs, userCount, &foundUserCount);

  for (auto saveInfo : saveInfoList) {
    bool accountPresent = false;

    for (s32 i = 0; i < foundUserCount; i++)
      if (userIDs[i] == saveInfo.uid)
        accountPresent = true;

    if (!accountPresent) continue;

    if (Title::g_titles.find(saveInfo.application_id) == Title::g_titles.end())
      Title::g_titles.insert({(u64)saveInfo.application_id, new Title(saveInfo)});

    Title::g_titles[saveInfo.application_id]->addUserID(saveInfo.uid);

    if (Account::g_accounts.find(saveInfo.uid) == Account::g_accounts.end()) {
      Account *account =  new Account(saveInfo.uid);

      if (!account->isInitialized()) {
        delete account;
        continue;
      }
      Account::g_accounts.insert(std::make_pair(static_cast<AccountUid>(saveInfo.uid), account));
    }
  }
}

void update() {
  while (updateThreadRunning) {
    auto begin = std::chrono::steady_clock::now();

    mutexLock(&mutexCurrGui);
    if (currGui != nullptr)
      currGui->update();
    mutexUnlock(&mutexCurrGui);

    if (kheld & (KEY_LEFT | KEY_RIGHT | KEY_UP | KEY_DOWN)) inputTicker++;
    else inputTicker = 0;

    svcSleepThread(1.0E6 - std::chrono::duration<double, std::nano>(std::chrono::steady_clock::now() - begin).count());
  }
}

void createFolders() {
  printf(EDIZON_DIR "/saves\n");
  mkdir("/switch", 0777);
  mkdir(EDIZON_DIR "", 0777);
  mkdir(EDIZON_DIR "/saves", 0777);
  mkdir(EDIZON_DIR "/batch_saves", 0777);
  mkdir(EDIZON_DIR "/restore", 0777);
  mkdir(EDIZON_DIR "/editor", 0777);
  mkdir(EDIZON_DIR "/editor/scripts", 0777);
  mkdir(EDIZON_DIR "/editor/scripts/lib", 0777);
  mkdir(EDIZON_DIR "/editor/scripts/lib/python3.5", 0777);
}

void requestDraw() {
  if (currGui != nullptr)
    currGui->draw();
}

void serviceInitialize() {
  setsysInitialize();
  socketInitializeDefault();
  nsInitialize();
  accountInitialize(AccountServiceType_Administrator);
  plInitialize();
  psmInitialize();
  pminfoInitialize();
  pmdmntInitialize();
  romfsInit();
  hidsysInitialize();
  pcvInitialize();
  clkrstInitialize();
  ledInit();

  curl_global_init(CURL_GLOBAL_ALL);

  u64 pid = 0;
  Title::g_activeTitle = 0;

  pmdmntGetApplicationProcessId(&pid);
  pminfoGetProgramId(&Title::g_activeTitle, pid);

  accountGetLastOpenedUser(&Account::g_activeUser);
}

void serviceExit() {
  setsysExit();
  socketExit();
  nsExit();
  accountExit();
  plExit();
  psmExit();
  pminfoExit();
  pmdmntExit();
  romfsExit();
  hidsysExit();
  pcvExit();
  clkrstExit();

  curl_global_cleanup();

  close(debugOutputFile);

}

void redirectStdio() {
  nxlinkStdio();

  debugOutputFile = open(EDIZON_DIR "/EdiZon.log", O_APPEND | O_WRONLY);

  if (debugOutputFile >= 0) {
    fflush(stdout);
    dup2(debugOutputFile, STDOUT_FILENO);
    fflush(stderr);
    dup2(debugOutputFile, STDERR_FILENO);
  }
}

int main(int argc, char** argv) {
  void *haddr;

  serviceInitialize();

  redirectStdio();

  framebufferCreate(&Gui::g_fb_obj, nwindowGetDefault(), 1280, 720, PIXEL_FORMAT_RGBA_8888, 2);
  framebufferMakeLinear(&Gui::g_fb_obj);

  ColorSetId colorSetId;
  setsysGetColorSetId(&colorSetId);
  setTheme(colorSetId);

  initTitles();

  printf("%s\n", EDIZON_DIR);

  createFolders();

  Config::readConfig();

  Gui::g_nextGui = GUI_MAIN;

  if (isServiceRunning("tx") && !isServiceRunning("rnx") && !Config::getConfig()->hideSX)
    Gui::g_nextGui = GUI_TX_WARNING;

  g_edizonPath = new char[strlen(argv[0]) + 1];
  strcpy(g_edizonPath, argv[0] + 5);

  mutexInit(&mutexCurrGui);

  updateThreadRunning = true;
  std::thread updateThread(update);


  while (appletMainLoop()) {
    hidScanInput();
    kheld = hidKeysHeld(CONTROLLER_P1_AUTO);
    kdown = hidKeysDown(CONTROLLER_P1_AUTO);

    if (Gui::g_nextGui != GUI_INVALID) {
      mutexLock(&mutexCurrGui);
      if (currGui != nullptr) {
        delete currGui;
        currGui = nullptr;
      }

      do {
        gui_t nextGuiStart = Gui::g_nextGui;
        switch (Gui::g_nextGui) {
          case GUI_MAIN:
            currGui = new GuiMain();
            break;
          case GUI_EDITOR:
            currGui = new GuiEditor();
            break;
          case GUI_TX_WARNING:
            currGui = new GuiTXWarning();
            break;
          case GUI_CHEATS:
            currGui = new GuiCheats();
            break;
          case GUI_GUIDE:
            currGui = new GuiGuide();
            break;
          case GUI_ABOUT:
            currGui = new GuiAbout();
            break;

          case GUI_INVALID: [[fallthrough]]
          default: break;
        }
        if (nextGuiStart == Gui::g_nextGui)
          Gui::g_nextGui = GUI_INVALID;
      } while(Gui::g_nextGui != GUI_INVALID);

      mutexUnlock(&mutexCurrGui);
    }

    if (currGui != nullptr) {
      currGui->draw();

      if (Gui::g_splashDisplayed) {
        if (inputTicker > LONG_PRESS_ACTIVATION_DELAY && (inputTicker % LONG_PRESS_DELAY) == 0) {
          if (Gui::g_currMessageBox != nullptr)
            Gui::g_currMessageBox->onInput(kheld);
          else if (Gui::g_currListSelector != nullptr)
            Gui::g_currListSelector->onInput(kheld);
          else
            currGui->onInput(kheld);
        } else if (kdown || hidKeysUp(CONTROLLER_P1_AUTO)) {
          if (Gui::g_currMessageBox != nullptr)
            Gui::g_currMessageBox->onInput(kdown);
          else if (Gui::g_currListSelector != nullptr)
            Gui::g_currListSelector->onInput(kdown);
          else
            currGui->onInput(kdown);
        }
      }
    }

    if (kheld != kheldOld) {
      inputTicker = 0;
    }

    static touchPosition touchPosStart, touchPosCurr, touchPosOld;
    static u8 touchCount, touchCountOld;
    static bool touchHappend = false;

    touchCount = hidTouchCount();

    if (touchCount > 0)
      hidTouchRead(&touchPosCurr, 0);

    if(touchCount > 0 && touchCountOld == 0)
      hidTouchRead(&touchPosStart, 0);

    if (abs(static_cast<s16>(touchPosStart.px - touchPosCurr.px)) < 10 && abs(static_cast<s16>(touchPosStart.py - touchPosCurr.py)) < 10) {
      if (touchCount == 0 && touchCountOld > 0) {
        touchHappend = true;

        if (Gui::g_currMessageBox != nullptr)
          Gui::g_currMessageBox->onTouch(touchPosCurr);
        else if (Gui::g_currListSelector != nullptr)
          Gui::g_currListSelector->onTouch(touchPosCurr);
        else
          currGui->onTouch(touchPosCurr);
      }
    } else if (touchCount > 0) {
      if (Gui::g_currMessageBox != nullptr)
        Gui::g_currMessageBox->onGesture(touchPosStart, touchPosCurr, false);
      else if (Gui::g_currListSelector != nullptr)
        Gui::g_currListSelector->onGesture(touchPosStart, touchPosCurr, false);
      else
        currGui->onGesture(touchPosStart, touchPosCurr, false);
    }

    if (touchCount == 0 && touchCountOld > 0 && !touchHappend) {
      if (Gui::g_currMessageBox != nullptr)
        Gui::g_currMessageBox->onGesture(touchPosStart, touchPosCurr, true);
      else if (Gui::g_currListSelector != nullptr)
        Gui::g_currListSelector->onGesture(touchPosStart, touchPosCurr, true);
      else
        currGui->onGesture(touchPosStart, touchPosCurr, true);
    }

    touchCountOld = touchCount;
    touchPosOld = touchPosCurr;
    touchHappend = false;

    kheldOld = kheld;

    if (Gui::g_requestExit) {
      if (Gui::g_currMessageBox == nullptr)
        break;
    }
  }

  updateThreadRunning = false;

  updateThread.join();

  delete[] g_edizonPath;

  for (auto it = Title::g_titles.begin(); it != Title::g_titles.end(); it++)
    delete it->second;

  for (auto it = Account::g_accounts.begin(); it != Account::g_accounts.end(); it++)
    delete it->second;

  Title::g_titles.clear();
  Account::g_accounts.clear();

  if (currGui != nullptr)
    delete currGui;

  framebufferClose(&Gui::g_fb_obj);

  serviceExit();

  return 0;
}
