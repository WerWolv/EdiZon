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
  
  typedef struct{
    u64 addr;
    MemoryType type;
  } ramAddr_t;

  typedef enum {
    UNSIGNED_8BIT,
    SIGNED_8BIT,
    UNSIGNED_16BIT,
    SIGNED_16BIT,
    UNSIGNED_32BIT,
    SIGNED_32BIT,
    UNSIGNED_64BIT,
    SIGNED_64BIT,
    FLOAT_32BIT,
    FLOAT_64BIT,
    POINTER,
    STRING
  } searchType_t;

  typedef enum {
    EQ,
    NEQ,
    GT,
    GTE,
    LT,
    LTE,
    RANGE,
    SAME,
    DIFF,
    INC,
    DEC
  } searchMode_t;

  typedef enum {
    HEAP,
    MAIN,
    HEAP_AND_MAIN,
    RAM
  } searchRange_t;

private:
  Debugger *m_debugger;
  u8 m_ramBuffer[0x10 * 14] = { 0 };
  color_t m_memory[1024] = { 0 };

  u8 m_selectedEntry = 0;
  enum { CHEATS, CANDIDATES } m_menuLocation = CHEATS;
  searchType_t m_searchType = UNSIGNED_8BIT;

  s128 m_searchValue;

  std::vector<MemoryInfo> m_memoryInfo;
  std::vector<ramAddr_t> m_foundAddresses;
  std::map<u64, u64> m_frozenAddresses;

  bool m_attached = false;
  bool m_sysmodulePresent = false;

  u64 m_addressSpaceBaseAddr = 0x00;
  u64 m_heapBaseAddr = 0x00;
  u64 m_codeBaseAddr = 0x00;
  u8 m_buildID[0x20];


  DmntCheatEntry *m_cheats;
  u64 m_cheatCnt;
};
