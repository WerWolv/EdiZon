#pragma once

#include "guis/gui.hpp"

#include <vector>
#include <unordered_map>
#include <stdbool.h>

#include "debugger.hpp"

class GuiRAMEditor : public Gui {
  
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

typedef struct {
  u64 addr;
  MemoryType type;
} ramAddr_t;

public:
  GuiRAMEditor();
  ~GuiRAMEditor();

  void update();
  void draw();
  void onInput(u32 kdown);
  void onTouch(touchPosition &touch);
  void onGesture(touchPosition startPosition, touchPosition endPosition, bool finish);

private:
  Debugger m_debugger;
  u8 m_ramBuffer[0x10 * 14] = { 0 };
  u64 m_ramAddress = 0;
  u8 m_selectedAddress = 0;

  std::vector<MemoryInfo> m_memoryInfo;
  color_t m_memory[1024] = { 0 };
  std::vector<ramAddr_t> m_foundAddresses;
  u8 m_searchType = SIGNED_8BIT;
  bool m_attached = false;
  bool m_sysmodulePresent = false;

  u64 m_addressSpaceBaseAddr = 0x00;
  u64 m_heapBaseAddr = 0x00;
  u64 m_codeBaseAddr = 0x00;

  enum {
    SEARCH_BEGIN,
    SEARCH_CONTINUE
  } m_searchMode;
};
