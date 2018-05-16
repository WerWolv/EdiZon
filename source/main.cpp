#include <switch.h>
#include <stdio.h>

#include "gui.hpp"
#include "gui_main.hpp"

extern "C" {
  #include "theme.h"
}

Gui *currGui = nullptr;

int main(int argc, char** argv) {
    gfxInitDefault();
    setsysInitialize();
    ColorSetId colorSetId;
    setsysGetColorSetId(&colorSetId);
    setTheme(colorSetId, 0x00000000, 0x00000000);

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
