#pragma once

#include "guis/gui.hpp"

#include <vector>
#include <unordered_map>
#include <stdbool.h>

#include "debugger.hpp"

class GuiRAMEditor : public Gui {
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
  std::vector<u64> m_foundAddresses;
  u8 m_searchType = SIGNED_8BIT;

  enum {
    SEARCH_BEGIN,
    SEARCH_CONTINUE
  } m_searchMode;

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
};
