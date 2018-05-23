#include <switch.h>
#include <stdio.h>
#include <vector>
#include <unordered_map>

#include "gui.hpp"
#include "gui_main.hpp"
#include "gui_editor.hpp"

#include "title.hpp"

extern "C" {
  #include "theme.h"
}

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

    if(accounts.find(saveInfo.userID) != accounts.end())
      accounts.insert({(u128)saveInfo.userID, new Account(saveInfo.userID)});
  }
}



int main(int argc, char** argv) {
  gfxInitDefault();
  setsysInitialize();
  ColorSetId colorSetId;
  setsysGetColorSetId(&colorSetId);
  setTheme(colorSetId, 0x00000000, 0x00000000);

  initTitles();

  Gui::g_nextGui = GUI_MAIN;

  while(appletMainLoop()) {
    hidScanInput();
    u32 kdown = hidKeysDown(CONTROLLER_P1_AUTO);

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
  }

  delete currGui;
  titles.clear();

  gfxExit();

  return 0;
}
