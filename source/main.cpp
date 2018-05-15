#include <switch.h>
#include <stdio.h>

#include "gui.hpp"

extern "C" {
  #include "theme.h"
}

int main(int argc, char** argv) {
    gfxInitDefault();
    setsysInitialize();
    ColorSetId colorSetId;
    HidSharedMemory *hidSharedMemory = (HidSharedMemory*) hidGetSharedmemAddr();
    setsysGetColorSetId(&colorSetId);
    setTheme(colorSetId, hidSharedMemory->controllers[0].header.singleColorBody, hidSharedMemory->controllers[1].header.singleColorBody);

    Gui gui;


    while(appletMainLoop()) {
        hidScanInput();
        u32 kdown = hidKeysDown(CONTROLLER_P1_AUTO);

        if(kdown & KEY_PLUS)
          break;

        gui.draw();
    }

    gfxExit();

    return 0;
}
