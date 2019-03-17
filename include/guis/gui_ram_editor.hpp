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

    typedef union { 
    u8 _u8;
    s8 _s8; 
    u16 _u16; 
    s16 _s16; 
    u32 _u32; 
    s32 _s32; 
    u64 _u64; 
    s64 _s64;
    float _f32;
    double _f64;
  } searchValue_t;

  typedef enum {
    SEARCH_TYPE_NONE = -1,
    SEARCH_TYPE_UNSIGNED_8BIT,
    SEARCH_TYPE_SIGNED_8BIT,
    SEARCH_TYPE_UNSIGNED_16BIT,
    SEARCH_TYPE_SIGNED_16BIT,
    SEARCH_TYPE_UNSIGNED_32BIT,
    SEARCH_TYPE_SIGNED_32BIT,
    SEARCH_TYPE_UNSIGNED_64BIT,
    SEARCH_TYPE_SIGNED_64BIT,
    SEARCH_TYPE_FLOAT_32BIT,
    SEARCH_TYPE_FLOAT_64BIT,
    SEARCH_TYPE_POINTER
  } searchType_t;

  typedef enum {
    SEARCH_MODE_NONE = -1,
    SEARCH_MODE_EQ,
    SEARCH_MODE_NEQ,
    SEARCH_MODE_GT,
    SEARCH_MODE_GTE,
    SEARCH_MODE_LT,
    SEARCH_MODE_LTE,
    SEARCH_MODE_RANGE,
    SEARCH_MODE_SAME,
    SEARCH_MODE_DIFF,
    SEARCH_MODE_INC,
    SEARCH_MODE_DEC
  } searchMode_t;

  typedef enum {
    SEARCH_REGION_NONE = -1,
    SEARCH_REGION_HEAP,
    SEARCH_REGION_MAIN,
    SEARCH_REGION_HEAP_AND_MAIN,
    SEARCH_REGION_RAM
  } searchRegion_t;

private:
  Debugger *m_debugger;
  u8 m_ramBuffer[0x10 * 14] = { 0 };
  color_t m_memory[1024] = { 0 };

  u8 m_selectedEntry = 0;
  
  searchValue_t m_searchValue;

  enum { CHEATS, CANDIDATES } m_menuLocation = CHEATS;
  enum { SEARCH_NONE, SEARCH_TYPE, SEARCH_VALUE, SEARCH_MODE, SEARCH_REGION } m_searchMenuLocation = SEARCH_NONE;
  searchType_t m_searchType = SEARCH_TYPE_NONE;
  searchMode_t m_searchMode = SEARCH_MODE_NONE;
  searchRegion_t m_searchRegion = SEARCH_REGION_NONE;

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

  void drawSearchRAMMenu();
};
