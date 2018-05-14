#include <switch.h>

#include "gui.hpp"

int main(int argc, char** argv) {
    gfxInitDefault();

    Gui gui;

    while(appletMainLoop() && !(hidKeysDown(CONTROLLER_P1_AUTO) & KEY_PLUS)) {
        hidScanInput();
        u32 kdown = hidKeysDown(CONTROLLER_P1_AUTO);

          gui.draw();
    }

    return 0;
}
