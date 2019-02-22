#include <switch.h>
#include <stratosphere.hpp>

#include "edizon_cheat_service.hpp"
#include "debugger.hpp"
#include "scripts.hpp"

#define HEAP_SIZE 0x00034000

static std::vector<u8> dataTypeSizes = { 1, 1, 2, 2, 4, 4, 8, 8, 4, 8, 8, 0 };

extern "C" {
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
}

struct CheatServerOptions {
    static constexpr size_t PointerBufferSize = 0x400;
    static constexpr size_t MaxDomains = 0;
    static constexpr size_t MaxDomainObjects = 0;
};

void freezeLoop(void *args) {
  (void)args;

  Debugger debugger;

  while(true) {
    mutexLock(&EdiZonCheatService::g_freezeMutex);

    if (EdiZonCheatService::g_frozenAddresses.size() == 0) {
      mutexUnlock(&EdiZonCheatService::g_freezeMutex);
      svcSleepThread(5E8L);
      continue;
    }

    if (debugger.getRunningApplicationPID() == 0)
      EdiZonCheatService::g_frozenAddresses.clear();

    debugger.attachToProcess();
    for (auto const& [addr, info] : EdiZonCheatService::g_frozenAddresses) {
      debugger.pokeMemory(dataTypeSizes[info.valueType], addr, info.freezeValue);
    } 
    debugger.detachFromProcess();

    mutexUnlock(&EdiZonCheatService::g_freezeMutex);
    svcSleepThread(5E8L);
  }
}

void scriptExecutionLoop(void *args) {
  Debugger debugger;
  Scripts scripts;

  while(true) {
    mutexLock(&EdiZonCheatService::g_freezeMutex);
    debugger.attachToProcess();

    for (auto & [fileName, cheat] : EdiZonCheatService::g_cheatScripts) {
      if (cheat.code == "") {
        FILE *cheatFile = fopen(std::string("/EdiZon/cheats/test.js").c_str(), "r");
        size_t fileSize = 0;
        char *buffer;

        fseek(cheatFile, 0, SEEK_END);
        fileSize = ftell(cheatFile);
        rewind(cheatFile);

        buffer = new char[fileSize];
        fread(buffer, 1, fileSize, cheatFile);
        fclose(cheatFile);

        cheat.code = buffer;
        delete[] buffer;
      }

      scripts.initScripts(cheat.code);

      for (auto const& [cheatName, enabled] : cheat.cheatNames) {
        if (!enabled) continue;

        scripts.executeScripts(cheatName);
      }

      scripts.finalizeScripts();
    }

    debugger.detachFromProcess();
    mutexUnlock(&EdiZonCheatService::g_freezeMutex);
    svcSleepThread(5E8L);
  }
}

int main(int argc, char **argv) {
  (void) argc;
  (void) argv;

  consoleDebugInit(debugDevice_SVC);

  mutexInit(&EdiZonCheatService::g_freezeMutex);

  Thread freezeThread, scriptExecThread;

  Result rc = threadCreate(&freezeThread, freezeLoop, NULL, 0x400, 49, 3);
  if (R_FAILED(rc))
    fatalSimple(rc);

  rc = threadCreate(&scriptExecThread, scriptExecutionLoop, NULL, 0x4000, 49, 3);
  if (R_FAILED(rc))
    fatalSimple(rc);

  rc = threadStart(&freezeThread);
  if (R_FAILED(rc))
    fatalSimple(rc);

  rc = threadStart(&scriptExecThread);
  if (R_FAILED(rc))
    fatalSimple(rc);


  auto serverManager = new WaitableManager<CheatServerOptions>(1);

  serverManager->AddWaitable(new ServiceServer<EdiZonCheatService>("edzn:cht", 10));

  serverManager->Process();

  threadClose(&freezeThread);
  delete serverManager;

  return 0;
}
