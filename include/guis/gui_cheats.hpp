#pragma once

#include "guis/gui.hpp"
#include "types.h"

#include <vector>
#include <set>
#include <unordered_map>
#include <stdbool.h>
#include <time.h>
#include "helpers/debugger.hpp"
#include "helpers/memory_dump.hpp"

#include "helpers/dmntcht.h"

enum
{
  FORMAT_DEC,
  FORMAT_HEX
} m_searchValueFormat = FORMAT_DEC;

class GuiCheats : public Gui
{

public:
  GuiCheats();
  ~GuiCheats();

  void update();
  void draw();
  void onInput(u32 kdown);
  void onTouch(touchPosition &touch);
  void onGesture(touchPosition startPosition, touchPosition endPosition, bool finish);

private:
  Debugger *m_debugger;
  u8 m_ramBuffer[0x10 * 14] = {0};
  color_t m_memory[1024] = {0};

  u8 m_selectedEntry = 0;
  u8 m_searchValueIndex = 0;
  u32 m_addresslist_offset = 0;

  searchValue_t m_searchValue[2];

  enum
  {
    CHEATS,
    CANDIDATES
  } m_menuLocation = CANDIDATES;
  enum
  {
    SEARCH_NONE,
    SEARCH_TYPE,
    SEARCH_MODE,
    SEARCH_REGION,
    SEARCH_VALUE,
    SEARCH_editRAM,
    SEARCH_POINTER
  } m_searchMenuLocation = SEARCH_NONE;

  searchType_t m_searchType = SEARCH_TYPE_NONE;
  searchMode_t m_searchMode = SEARCH_MODE_NONE;
  searchRegion_t m_searchRegion = SEARCH_REGION_NONE;

  std::vector<MemoryInfo> m_memoryInfo;
  std::vector<MemoryInfo> m_targetmemInfos;

  MemoryDump *m_memoryDump;
  MemoryDump *m_memoryDump1;
  MemoryDump *m_memoryDumpBookmark;
  MemoryDump *m_AttributeDumpBookmark;
  MemoryDump *m_pointeroffsetDump;
  MemoryDump *m_dataDump;
  struct PSsetup_t
  {
    u64 m_numoffset = 3;
    u64 m_max_source = 200;
    u64 m_max_depth = 2;
    u64 m_max_range = 0x300;
    u64 m_EditorBaseAddr = 0;
    u64 m_mainBaseAddr;
    u64 m_mainend;
    bool m_pointersearch_canresume = false;
    bool m_PS_resume = false;
    bool m_PS_pause = false;
    // PointerSearch_state *m_PointerSearch = nullptr;
  };
  u64 m_target = 0;
  u64 m_numoffset = 3;
  u64 m_max_source = 200;
  u64 m_max_depth = 2;
  u64 m_max_range = 0x300;
  u64 m_low_main_heap_addr = 0x100000000;
  u64 m_high_main_heap_addr = 0x10000000000;
  u64 m_pointer_found = 0;
  bool m_abort = false;
  bool m_showpointermenu = false;

  std::map<u64, u64> m_frozenAddresses;

  bool m_cheatsPresent = false;
  bool m_sysmodulePresent = false;
  bool m_editRAM = false;
  bool m_nothingchanged = false;

  u64 m_addressSpaceBaseAddr = 0x00;
  u64 m_addressSpaceSize = 0x00;
  u64 m_heapBaseAddr = 0x00;
  u64 m_mainBaseAddr = 0x00;
  u64 m_EditorBaseAddr = 0x00;
  u8 m_addressmod = 0;
  time_t m_Time1;
  struct helperinfo_t
  {
    u64 address;
    u64 size;
    u64 count;
  };
  struct sourceinfo_t
  {
    u64 foffset;
    u64 offset;
  };

#define IS_PTRCODE_START 0x580F0000
#define IS_OFFSET 0x580F1000
#define IS_FINAL 0x780F0000

#define MAX_NUM_SOURCE_POINTER 200 // bound check for debugging;
#define MAX_POINTER_DEPTH 12       // up to 4 seems OK with forward only search took 94s. 215s for big dump
#define MAX_POINTER_RANGE 0x2000
#define MAX_NUM_POINTER_OFFSET 30
#define HAVESAVE false
  struct PointerSearch_state
  {
    u64 depth = 0;                                                       // depth and index[depth] is where the search is at, pointersearch2 will increment depth and call itself with nexttarget
    u64 index[MAX_POINTER_DEPTH + 1] = {0};                              // when there is a hit retrieve the offsets with indexs and sources[index].offset
    u64 num_offsets[MAX_POINTER_DEPTH + 1] = {0};                        // for analysis
    u64 num_sources[MAX_POINTER_DEPTH + 1] = {0};                        // this is how we will go down the column
    sourceinfo_t sources[MAX_POINTER_DEPTH + 1][MAX_NUM_SOURCE_POINTER]; //the data matrix to get nexttarget from foffset and m_memoryDump1
  };
  PointerSearch_state *m_PointerSearch = nullptr;
  void startpointersearch2(u64 targetaddress);
  void pointersearch2(u64 targetaddress, u64 depth);
  void resumepointersearch2();
  bool m_pointersearch_canresume = false;
  bool m_PS_resume = false;
  bool m_PS_pause = false;
#define PS_depth depth
#define PS_index m_PointerSearch->index[PS_depth]
// #define PS_indexP m_PointerSearch->index[PS_depth-1]
#define PS_num_offsets m_PointerSearch->num_offsets[PS_depth]
#define PS_num_sources m_PointerSearch->num_sources[PS_depth]
// #define PS_num_sourcesP m_PointerSearch->num_sources[PS_depth-1]
#define PS_sources m_PointerSearch->sources[PS_depth]
#define PS_lastdepth m_PointerSearch->depth
#define REPLACEFILE(file1, file2)             \
  remove(file2);                              \
  while (access(file2, F_OK) == 0)            \
  {                                           \
    printf("waiting for delete %s\n", file2); \
  }                                           \
  rename(file1, file2);                       \
  while (access(file2, F_OK) != 0)            \
  {                                           \
    printf("waiting for rename %s\n", file1); \
  }

  bool m_forwardonly = false;
  bool m_forwarddump = false; // reduce from 138 to 26
  struct pointer_chain_t
  {
    u64 depth = 0;
    u64 offset[MAX_POINTER_DEPTH + 1] = {0}; // offset to address pointed by pointer
    // below is for debugging can consider removing;
    // u64 fileoffset[MAX_POINTER_DEPTH + 1] = {0}; // offset to the file that has the address where the pointer was stored in this instance for debugging
  };
  struct bookmark_t
  {
    char label[18] = {0};
    searchType_t type;
    pointer_chain_t pointer;
    bool heap = true;
    u32 offset = 0;
    bool deleted = false;
  };
  bookmark_t m_bookmark;      //global for use in pointer search , target address to be updated dynamically by display routine TBD
  bookmark_t bookmark;        //used in add book mark
  pointer_chain_t m_hitcount; // maybe not used

  std::stringstream m_PCDump_filename;
  void PCdump();
  enum MemoryType
  {
    MAIN,
    HEAP
  };
  void rebasepointer(searchValue_t value); //struct bookmark_t bookmark);
  // bool check_chain(bookmark_t *bookmark, u64 *address);
  // void startpointersearch(u64 address, u64 depth, u64 range, u64 num, MemoryDump **displayDump);
  // void startpointersearch(u64 address, u64 depth, u64 range, u64 num, MemoryDump **displayDump);
  // void pointersearch(u64 targetaddress, MemoryDump **displayDump, MemoryDump **dataDump, pointer_chain_t pointerchain);
  void pointersearch(u64 targetaddress, struct pointer_chain_t pointerchain);
  void pointercheck();
  void startpointersearch(u64 targetaddress); //, MemoryDump **displayDump);
  void searchpointer(u64 address, u64 depth, u64 range, struct pointer_chain_t pointerchain);
  bool valuematch(searchValue_t value, u64 nextaddress);
  bool getinput(std::string headerText, std::string subHeaderText, std::string initialText, searchValue_t *searchValue);
  bool addcodetofile(u64 index);
  bool addstaticcodetofile(u64 index);
  void PSsaveSTATE();
  void PSresumeSTATE();
  void updatebookmark(bool clearunresolved, bool importbookmark);
  bool unresolved(pointer_chain_t pointer);
  bool unresolved2(pointer_chain_t *pointer);
  u64 m_heapSize = 0;
  u64 m_mainSize = 0;
  u64 m_heapEnd = 0;
  u64 m_mainend = 0;
  u8 m_buildID[0x20];

  DmntCheatEntry *m_cheats;
  u64 m_cheatCnt;

  void drawSearchRAMMenu();

  void drawEditRAMMenu();
  void drawSearchPointerMenu();
  void searchMemoryAddressesPrimary(Debugger *debugger, searchValue_t searchValue1,
                                    searchValue_t searchValue2, searchType_t searchType,
                                    searchMode_t searchMode, searchRegion_t searchRegion,
                                    MemoryDump **displayDump, std::vector<MemoryInfo> memInfos);

  void searchMemoryAddressesPrimary2(Debugger *debugger, searchValue_t searchValue1,
                                     searchValue_t searchValue2, searchType_t searchType,
                                     searchMode_t searchMode, searchRegion_t searchRegion,
                                     MemoryDump **displayDump, std::vector<MemoryInfo> memInfos);

  void searchMemoryAddressesSecondary(Debugger *debugger, searchValue_t searchValue1,
                                      searchValue_t searchValue2, searchType_t searchType,
                                      searchMode_t searchMode, MemoryDump **displayDump);

  void searchMemoryValuesPrimary(Debugger *debugger, searchType_t searchType, searchMode_t searchMode,
                                 searchRegion_t searchRegion, MemoryDump **displayDump, std::vector<MemoryInfo> memInfos);

  void searchMemoryValuesSecondary(Debugger *debugger, searchType_t searchType,
                                   searchMode_t searchMode, searchRegion_t searchRegion,
                                   MemoryDump **displayDump, std::vector<MemoryInfo> memInfos);

  void searchMemoryValuesTertiary(Debugger *debugger, searchValue_t searchValue1,
                                  searchValue_t searchValue2, searchType_t searchType,
                                  searchMode_t searchMode, searchRegion_t searchRegion,
                                  MemoryDump **displayDump, std::vector<MemoryInfo> memInfos);
};
