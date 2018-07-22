#include <switch.h>
#include <stdio.h>
#include <vector>
#include <algorithm>

#include "gui.hpp"
#include "gui_main.hpp"
#include "gui_editor.hpp"

#include "title.hpp"

extern "C" {
  #include "theme.h"
}

#define NXLINK

#define LONG_PRESS_DELAY              2
#define LONG_PRESS_ACTIVATION_DELAY   10

size_t __nx_heap_size = 0x200000 * 32;

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

int main(int argc, char** argv) {
  u8 touchCntOld, touchCnt;
  u32 kheld = 0, kheldOld = 0;
  u32 kdown = 0;
  touchPosition touch;
  touchPosition touchEnd;

  s32 inputTicker = 0;

  socketInitializeDefault();

#ifdef NXLINK
  nxlinkStdio();
#endif

  gfxInitDefault();
  setsysInitialize();
  ColorSetId colorSetId;
  setsysGetColorSetId(&colorSetId);
  setTheme(colorSetId);

  initTitles();

  Gui::g_nextGui = GUI_MAIN;
  touchCntOld = hidTouchCount();

  while (appletMainLoop()) {
    hidScanInput();
    kdown = hidKeysDown(CONTROLLER_P1_AUTO);
    kheld = hidKeysHeld(CONTROLLER_P1_AUTO);

    if (kdown & KEY_PLUS)
      break;

    if (Gui::g_nextGui != GUI_INVALID) {
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
    }

    currGui->draw();

    if (kdown || hidKeysUp(CONTROLLER_P1_AUTO)) {
      if (Gui::g_currMessageBox != nullptr)
        Gui::g_currMessageBox->onInput(kdown);
      else if (Gui::g_currListSelector != nullptr)
        Gui::g_currListSelector->onInput(kdown);
      else
        currGui->onInput(kdown);
    }

    if (kheld & (KEY_LEFT | KEY_RIGHT | KEY_UP | KEY_DOWN)) inputTicker++;
    else inputTicker = 0;

    if (kheld != kheldOld)
      inputTicker = 0;

    if (inputTicker > LONG_PRESS_ACTIVATION_DELAY && (inputTicker % LONG_PRESS_DELAY) == 0)
      currGui->onInput(kheld);

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
  }

  delete currGui;

  for (auto it = Title::g_titles.begin(); it != Title::g_titles.end(); it++)
    delete it->second;

  for (auto it = Account::g_accounts.begin(); it != Account::g_accounts.end(); it++)
    delete it->second;

  Title::g_titles.clear();
  Account::g_accounts.clear();

  socketExit();
  gfxExit();

  return 0;
}
