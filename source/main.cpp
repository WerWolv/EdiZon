#include <switch.h>
#include <stdio.h>
#include <vector>
#include <unordered_map>

#include "gui.hpp"
#include "gui_main.hpp"

#include "title.hpp"

extern "C" {
  #include "theme.h"
}

Gui *currGui = nullptr;
std::unordered_map<uint64_t, void*> titles;

void initTitles() {
  std::vector<FsSaveDataInfo> saveInfoList;
  _getSaveList(saveInfoList);

  for(auto saveInfo : saveInfoList) {
    if(titles.find(saveInfo.titleID) == titles.end())
      titles.insert(std::pair<uint64_t, void*>((uint64_t)saveInfo.titleID, new Title(saveInfo)));

     ((Title*)titles[saveInfo.titleID])->addUserID(saveInfo.userID);
  }
}

int main(int argc, char** argv) {
    gfxInitDefault();
    setsysInitialize();
    ColorSetId colorSetId;
    setsysGetColorSetId(&colorSetId);
    setTheme(colorSetId, 0x00000000, 0x00000000);

    initTitles();

    currGui = new GuiMain();

    while(appletMainLoop()) {
        hidScanInput();
        u32 kdown = hidKeysDown(CONTROLLER_P1_AUTO);

        if(kdown & KEY_PLUS)
          break;

        currGui->draw();
    }

    delete currGui;

    gfxExit();

    return 0;
}
