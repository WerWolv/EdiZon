#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <switch.h>

#define TITLE_ID 0x01000000000001FF
#define HEAP_SIZE 0x000340000

u32 __nx_applet_type = AppletType_None;

char fake_heap[HEAP_SIZE];

void __libnx_initheap(void) {
    extern char *fake_heap_start;
    extern char *fake_heap_end;

    // setup newlib fake heap
    fake_heap_start = fake_heap;
    fake_heap_end = fake_heap + HEAP_SIZE;
}

void registerFspLr() {
    if (kernelAbove400())
        return;

    Result rc = fsprInitialize();
    if (R_FAILED(rc))
        fatalSimple(rc);

    u64 pid;
    svcGetProcessId(&pid, CUR_PROCESS_HANDLE);

    rc = fsprRegisterProgram(pid, TITLE_ID, FsStorageId_NandSystem, NULL, 0, NULL, 0);
    if (R_FAILED(rc))
        fatalSimple(rc);
    fsprExit();
}

void __appInit(void) {
    Result rc;
    svcSleepThread(20000000000L);
    rc = smInitialize();
    if (R_FAILED(rc))
        fatalSimple(rc);
    rc = fsInitialize();
    if (R_FAILED(rc))
        fatalSimple(rc);
    registerFspLr();
    rc = fsdevMountSdmc();
    if (R_FAILED(rc))
        fatalSimple(rc);
    rc = timeInitialize();
    if (R_FAILED(rc))
        fatalSimple(rc);
    rc = socketInitializeDefault();
    if (R_FAILED(rc))
        fatalSimple(rc);
}

void __appExit(void) {
    fsdevUnmountAll();
    fsExit();
    smExit();
    audoutExit();
    timeExit();
    socketExit();
}

int main() {

    while (appletMainLoop()) {
		
	}

    return 0;
}
