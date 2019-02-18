#include <switch.h>
#include <stratosphere.hpp>

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

void __appInit(void) {
  Result rc;

  rc = smInitialize();
  if (R_FAILED(rc))
    fatalSimple(rc);
  rc = fsInitialize();
  if (R_FAILED(rc))
    fatalSimple(rc);
  rc = fsdevMountSdmc();
  if (R_FAILED(rc))
    fatalSimple(rc);
}

void __appExit(void) {
  fsdevUnmountAll();
  fsExit();
  smExit();
}

int main(int argc, char **argv) {

  while (appletMainLoop()) {
		
	}

  return 0;
}
