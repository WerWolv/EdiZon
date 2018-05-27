#include <switch.h>
#include <stdio.h>
#include <vector>
#include <unordered_map>
#include <algorithm>

#include "gui.hpp"
#include "gui_main.hpp"
#include "gui_editor.hpp"

#include "title.hpp"

extern "C" {
  #include "theme.h"
}

#define LONG_PRESS_DELAY              2
#define LONG_PRESS_ACTIVATION_DELAY   10

std::unordered_map<u64, Title*> titles;
std::unordered_map<u128, Account*> accounts;

Gui* currGui = nullptr;

void initTitles() {
  std::vector<FsSaveDataInfo> saveInfoList;
  _getSaveList(saveInfoList);

  for(auto saveInfo : saveInfoList) {
    if(titles.find(saveInfo.titleID) == titles.end())
      titles.insert({(u64)saveInfo.titleID, new Title(saveInfo)});
    titles[saveInfo.titleID]->addUserID(saveInfo.userID);

    if(accounts.find(saveInfo.userID) == accounts.end())
      accounts.insert({(u128)saveInfo.userID, new Account(saveInfo.userID)});
  }
}

int main(int argc, char** argv) {
  u8 touchCntOld, touchCnt;
  u32 kheld = 0, kheldOld = 0;
  u32 kdown = 0;
  s32 inputTicker = 0;


  gfxInitDefault();
  setsysInitialize();
  ColorSetId colorSetId;
  setsysGetColorSetId(&colorSetId);
  setTheme(colorSetId, 0x00000000, 0x00000000);

  initTitles();

  Gui::g_nextGui = GUI_MAIN;
  touchCntOld = hidTouchCount();

  while(appletMainLoop()) {
    hidScanInput();
    kdown = hidKeysDown(CONTROLLER_P1_AUTO);
    kheld = hidKeysHeld(CONTROLLER_P1_AUTO);

    if(kdown & KEY_PLUS)
      break;

    if(Gui::g_nextGui != GUI_INVALID) {
      delete currGui;
      switch(Gui::g_nextGui) {
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

    if(kdown)
      currGui->onInput(kdown);

    if(kheld & (KEY_LEFT | KEY_RIGHT | KEY_UP | KEY_DOWN)) inputTicker++;
    else inputTicker = 0;

    if(kheld != kheldOld)
      inputTicker = 0;

    if(inputTicker > LONG_PRESS_ACTIVATION_DELAY && (inputTicker % LONG_PRESS_DELAY) == 0)
      currGui->onInput(kheld);



    touchCnt = hidTouchCount();

    if(touchCnt > touchCntOld) {
      touchPosition touch;
      hidTouchRead(&touch, 0);
      currGui->onTouch(touch);
    }

    touchCntOld = touchCnt;
    kheldOld = kheld;
  }

  delete currGui;
  titles.clear();

  gfxExit();

  return 0;
}
