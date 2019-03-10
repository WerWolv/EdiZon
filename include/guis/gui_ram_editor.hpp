#pragma once

#include "guis/gui.hpp"

#include <vector>
#include <set>
#include <unordered_map>
#include <stdbool.h>

#include "debugger.hpp"

extern "C" {
  #include "dmntcht.h"
}


class GuiRAMEditor : public Gui {

public:
  GuiRAMEditor();
  ~GuiRAMEditor();

  void update();
  void draw();
  void onInput(u32 kdown);
  void onTouch(touchPosition &touch);
  void onGesture(touchPosition startPosition, touchPosition endPosition, bool finish);

  typedef struct {
  u64 addr;
  MemoryType type;
} ramAddr_t;

  typedef enum {
  SIGNED_8BIT,
  UNSIGNED_8BIT,
  SIGNED_16BIT,
  UNSIGNED_16BIT,
  SIGNED_32BIT,
  UNSIGNED_32BIT,
  SIGNED_64BIT,
  UNSIGNED_64BIT,
  FLOAT_32BIT,
  FLOAT_64BIT,
  POINTER,
  STRING
  } searchType_t;

  typedef enum {
    SEARCH_BEGIN,
    SEARCH_CONTINUE
  } searchMode_t;

private:
  Debugger *m_debugger;
  u8 m_ramBuffer[0x10 * 14] = { 0 };
  color_t m_memory[1024] = { 0 };

  u8 m_selectedEntry = 0;
  enum { CHEATS, CANDIDATES } m_menuLocation = CHEATS;
  u8 m_searchType = SIGNED_8BIT;

  std::vector<MemoryInfo> m_memoryInfo;
  std::vector<ramAddr_t> m_foundAddresses;
  std::set<u64> m_frozenAddresses;

  bool m_attached = false;
  bool m_sysmodulePresent = false;

  u64 m_addressSpaceBaseAddr = 0x00;
  u64 m_heapBaseAddr = 0x00;
  u64 m_codeBaseAddr = 0x00;
  u8 m_buildID[0x20];

  searchMode_t m_searchMode;

  DmntCheatEntry *m_cheats;
  u64 m_cheatCnt;
};
