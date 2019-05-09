#include <switch.h>
#include <stdio.h>
#include <vector>
#include <algorithm>
#include <chrono>
#include <thread>

#include <iostream>
#include <unistd.h>
#include <fcntl.h>

#include "guis/gui.hpp"
#include "guis/gui_main.hpp"
#include "guis/gui_editor.hpp"
#include "guis/gui_tx_warning.hpp"
#include "guis/gui_cheats.hpp"
#include "guis/gui_information.hpp"

#include "helpers/title.hpp"

#include "theme.h"
#include "helpers/util.h"

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
  size_t foundUserCount = 0;
  
  accountGetUserCount(&userCount);

  u128 userIDs[userCount];
  accountListAllUsers(userIDs, userCount, &foundUserCount);

  for (auto saveInfo : saveInfoList) {
    bool accountPresent = false;

    for (u32 i = 0; i < foundUserCount; i++)
      if (userIDs[i] == saveInfo.userID)
        accountPresent = true;

    if (!accountPresent) continue;

    if (Title::g_titles.find(saveInfo.titleID) == Title::g_titles.end())
      Title::g_titles.insert({(u64)saveInfo.titleID, new Title(saveInfo)});
      
    Title::g_titles[saveInfo.titleID]->addUserID(saveInfo.userID);

    if (Account::g_accounts.find(saveInfo.userID) == Account::g_accounts.end()) {
      Account *account =  new Account(saveInfo.userID);

      if (!account->isInitialized()) {
        delete account;
        continue;
      }
      Account::g_accounts.insert(std::make_pair(static_cast<u128>(saveInfo.userID), account));
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
  mkdir("/switch", 0777);
  mkdir("/switch/EdiZon", 0777);
  mkdir("/switch/EdiZon/saves", 0777);
  mkdir("/switch/EdiZon/batch_saves", 0777);
  mkdir("/switch/EdiZon/restore", 0777);
  mkdir("/switch/EdiZon/editor", 0777);
  mkdir("/switch/EdiZon/editor/scripts", 0777);
  mkdir("/switch/EdiZon/editor/scripts/lib", 0777);
  mkdir("/switch/EdiZon/editor/scripts/lib/python3.5", 0777);
}

void requestDraw() {
  if (currGui != nullptr)
    currGui->draw();
}

void serviceInitialize() {
  setsysInitialize();
  socketInitializeDefault();
  accountInitialize();
  nsInitialize();
  plInitialize();
  psmInitialize();
  pmdmntInitialize();
  pminfoInitialize();
  romfsInit();
  hidsysInitialize();
  pcvInitialize();
  clkrstInitialize();
  ledInit();

  u64 pid = 0;
  u128 activeUser;
  bool accountSelected;
  Title::g_activeTitle = 0;

  pmdmntGetApplicationPid(&pid);
  pminfoGetTitleId(&Title::g_activeTitle, pid);

  accountGetActiveUser(&activeUser, &accountSelected);

  if (accountSelected)
    Account::g_activeUser = activeUser;
}

void serviceExit() {
  socketExit();
  accountExit();
  nsExit();
  plExit();
  psmExit();
  pminfoExit();
  pmdmntExit();
  romfsExit();
  setsysExit();
  hidsysExit();
  pcvExit();
  clkrstExit();

  close(debugOutputFile);

}

void redirectStdio() {
  nxlinkStdio();

  debugOutputFile = open("/switch/EdiZon/EdiZon.log", O_APPEND | O_WRONLY);

  if (debugOutputFile >= 0) {
    fflush(stdout);
    dup2(debugOutputFile, STDOUT_FILENO);
    fflush(stderr);
    dup2(debugOutputFile, STDERR_FILENO);
  }
}

int main(int argc, char** argv) {
  void *haddr;
  extern char *fake_heap_end;
  
  // Setup Heap for swkbd on applets
  Result rc = svcSetHeapSize(&haddr, 0x10000000);
  if (R_FAILED(rc))
    fatalSimple(0xDEAD);
  fake_heap_end = (char*) haddr + 0x10000000;

  serviceInitialize();

  redirectStdio();

  framebufferCreate(&Gui::g_fb_obj, nwindowGetDefault(), 1280, 720, PIXEL_FORMAT_RGBA_8888, 2);
  framebufferMakeLinear(&Gui::g_fb_obj);

  ColorSetId colorSetId;
  setsysGetColorSetId(&colorSetId);
  setTheme(colorSetId);

  initTitles();

  createFolders();

  Gui::g_nextGui = GUI_MAIN;

  if (isServiceRunning("tx") && !isServiceRunning("rnx") && access("/switch/EdiZon/.hide_sxos", F_OK) == -1)
    Gui::g_nextGui = GUI_TX_WARNING;

  if (access("/switch/EdiZon/memdump.dat", F_OK) == 0)
    Gui::g_nextGui = GUI_CHEATS;

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
            currGui = new GuiRAMEditor();
            break;
          case GUI_INFORMATION:
            currGui = new GuiInformation();
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

  svcSetHeapSize(&haddr, ((u8*) envGetHeapOverrideAddr() + envGetHeapOverrideSize()) - (u8*) haddr); // clean up the heap

  return 0;
}
