#include <switch.h>

int main(int argc, char** argv)
{
    while(appletMainLoop() && !(hidKeysDown(CONTROLLER_P1_AUTO) & KEY_PLUS))
    {
        hidScanInput();
        u32 kdown = hidKeysDown(CONTROLLER_P1_AUTO);

        
    }

    return 0;
}