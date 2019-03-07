#include "guis/gui_ram_editor.hpp"

#include <sstream>
#include <limits>
#include <utility>

#include <bits/stdc++.h>

extern "C" {
  #include "util.h"
}

static const std::vector<std::string> dataTypes = { "s8", "u8", "s16", "u16", "s32", "u32", "s64", "u64", "f32", "f64", "ptr", "str" };
static const std::vector<u8> dataTypeSizes      = {    1,   1,     2,     2,     4,     4,     8,     8,     4,     8,     8,     0  };
static const std::vector<s128> dataTypeMaxValues = { std::numeric_limits<s8>::max(), std::numeric_limits<u8>::max(), std::numeric_limits<s16>::max(), std::numeric_limits<u16>::max(), std::numeric_limits<s32>::max(), std::numeric_limits<u32>::max(), std::numeric_limits<s64>::max(), std::numeric_limits<u64>::max(), std::numeric_limits<s32>::max(), std::numeric_limits<s64>::max(), std::numeric_limits<u64>::max() };
static const std::vector<s128> dataTypeMinValues = { std::numeric_limits<s8>::min(), std::numeric_limits<u8>::min(), std::numeric_limits<s16>::min(), std::numeric_limits<u16>::min(), std::numeric_limits<s32>::min(), std::numeric_limits<u32>::min(), std::numeric_limits<s64>::min(), std::numeric_limits<u64>::min(), std::numeric_limits<s32>::min(), std::numeric_limits<s64>::min(), std::numeric_limits<u64>::min() };

static std::string titleNameStr, tidStr, pidStr;


bool isAddressFrozen(uintptr_t address);
std::string getAddressDisplayString(GuiRAMEditor::ramAddr_t address, Debugger *debugger, GuiRAMEditor::searchType_t searchType, u64 heapBaseAddr, u64 codeBaseAddr, u64 addressSpaceBaseAddr);


GuiRAMEditor::GuiRAMEditor() : Gui() {
  m_searchMode = SEARCH_BEGIN;
  m_searchType = SIGNED_8BIT;

  m_sysmodulePresent = isServiceRunning("dmnt:cht");

  m_debugger = new Debugger(m_sysmodulePresent);

  if (m_debugger->getRunningApplicationPID() == 0) {
    remove("/EdiZon/addresses.dat");
    return;
  }

  m_cheatCnt = 0;

  if (m_sysmodulePresent) {
    dmntchtInitialize();
    dmntchtForceOpenCheatProcess();

    DmntCheatProcessMetadata metadata;
    dmntchtGetCheatProcessMetadata(&metadata);

    m_addressSpaceBaseAddr = metadata.address_space_extents.base;
    m_heapBaseAddr = metadata.heap_extents.base;
    m_codeBaseAddr = metadata.main_nso_extents.base;

    u64 cheatCnt = 0;

    dmntchtGetCheatCount(&cheatCnt);

    if (cheatCnt > 0) {
      m_cheats = new DmntCheatEntry[cheatCnt];
      dmntchtGetCheats(m_cheats, cheatCnt, 0, &m_cheatCnt);
    }

    u64 addressCnt = 0;
    dmntchtGetFrozenAddressCount(&addressCnt);

    if (addressCnt != 0) {
      DmntFrozenAddressEntry frozenAddresses[addressCnt];
      dmntchtGetFrozenAddresses(frozenAddresses, addressCnt, 0, nullptr);
      
      for (u16 i = 0; i < addressCnt; i++)
        m_frozenAddresses.insert(frozenAddresses[i].address);

    }
  } else {
    Handle appHandle = INVALID_HANDLE;

    amspmdmntInitialize();
    amspmdmntAtmosphereGetProcessHandle(&appHandle);

    if (appHandle != INVALID_HANDLE) {
      svcGetInfo(&m_addressSpaceBaseAddr, 12, appHandle, 0);
      svcGetInfo(&m_heapBaseAddr, 4, appHandle, 0);
      m_codeBaseAddr = 0x00;
    }
  }  

  m_attached = true;

  MemoryInfo meminfo = { 0 };
  u64 lastAddr = 0;

  do {
    lastAddr = meminfo.addr;
    meminfo = m_debugger->queryMemory(meminfo.addr + meminfo.size);

    m_memoryInfo.push_back(meminfo);
  } while (lastAddr < meminfo.addr + meminfo.size);

  m_debugger->continueProcess();

  for (MemoryInfo meminfo : m_memoryInfo) {
    if (m_codeBaseAddr == 0x00 && (meminfo.type == MemType_CodeStatic))
      m_codeBaseAddr = meminfo.addr;

    for (u64 addrOffset = meminfo.addr; addrOffset < meminfo.addr + meminfo.size; addrOffset += 0x20000000) {
      switch(meminfo.type) {
        case MemType_CodeStatic: 
        case MemType_CodeMutable: m_memory[addrOffset / 0x20000000] = Gui::makeColor(0xFF, 0x00, 0x00, 0xFF); break;
        case MemType_SharedMem: m_memory[addrOffset / 0x20000000] = Gui::makeColor(0x00, 0xFF, 0x00, 0xFF); break;
        case MemType_Heap: m_memory[addrOffset / 0x20000000] = Gui::makeColor(0x00, 0x00, 0xFF, 0xFF); break;
        case MemType_KernelStack:
        case MemType_ThreadLocal: m_memory[addrOffset / 0x20000000] = Gui::makeColor(0xFF, 0xFF, 0x00, 0xFF); break;
        case MemType_Unmapped: break;
        default: m_memory[addrOffset / 0x20000000] = Gui::makeColor(0x80, 0x80, 0x80, 0xFF); break;
      }
    }
  }

  u64 oldHeapBaseAddr = 0;
  size_t addressCnt = 0;
  std::ifstream file("/EdiZon/addresses.dat", std::ios::in | std::ios::binary);

  if (file.is_open()) {
    m_searchMode = SEARCH_CONTINUE;
    file.read((char*)&addressCnt, 8);
    file.read((char*)&m_searchType, 1);
    file.read((char*)&oldHeapBaseAddr, 8);

    ramAddr_t *buffer = new ramAddr_t[addressCnt];
    m_foundAddresses.reserve(addressCnt);

    file.read((char*)buffer, addressCnt * sizeof(ramAddr_t));

    std::copy(buffer, buffer + addressCnt, std::back_inserter(m_foundAddresses));

    delete[] buffer;

    file.close();
  }

  if (m_heapBaseAddr != oldHeapBaseAddr && oldHeapBaseAddr != 0) {
    m_searchMode = SEARCH_BEGIN;
    remove("/EdiZon/addresses.dat");
    m_foundAddresses.clear();
    m_searchType = 0;
    Gui::g_nextGui = GUI_MAIN;
  }

  std::stringstream ss;
  if (Title::g_titles[m_debugger->getRunningApplicationTID()] != nullptr) {
    if (Title::g_titles[m_debugger->getRunningApplicationTID()]->getTitleName().length() < 24)
      ss << Title::g_titles[m_debugger->getRunningApplicationTID()]->getTitleName();
    else
      ss << Title::g_titles[m_debugger->getRunningApplicationTID()]->getTitleName().substr(0, 21) << "...";
    titleNameStr = ss.str();
    ss.str("");
  } else titleNameStr = "Unknown title name!";

  ss << "TID: " << std::uppercase << std::hex << std::setfill('0') << std::setw(sizeof(u64) * 2) << m_debugger->getRunningApplicationTID();
  tidStr = ss.str();
  ss.str("");

  ss << "PID: " << std::dec << m_debugger->getRunningApplicationPID();
  pidStr = ss.str();

  if (!m_sysmodulePresent) {
    (new MessageBox("Atmosphère's cheat module is not running on this System. \n Cheat management and variable freezing is disabled.", MessageBox::OKAY))->show();
  }

  if (m_cheatCnt == 0)
    m_menuLocation = CANDIDATES;
  if (m_foundAddresses.size() == 0)
    m_menuLocation = CHEATS;
}

GuiRAMEditor::~GuiRAMEditor() {
  m_debugger->detachFromProcess();

  if (m_sysmodulePresent)
    amspmdmntExit();
}



void GuiRAMEditor::update() {
  Gui::update();
}

void GuiRAMEditor::draw() {
  std::stringstream ss;

  Gui::beginDraw();

  Gui::drawRectangle(0, 0, Gui::g_framebuffer_width, Gui::g_framebuffer_height, currTheme.backgroundColor);

  Gui::drawRectangle((u32)((Gui::g_framebuffer_width - 1220) / 2), Gui::g_framebuffer_height - 73, 1220, 1, currTheme.textColor);


  if (m_debugger->getRunningApplicationPID() == 0) {
    Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2 - 50, currTheme.textColor, "A title needs to be running in the background to use the RAM editor. \n Please launch an application and try again.", ALIGNED_CENTER);
    Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 50, Gui::g_framebuffer_height - 50, currTheme.textColor, "\uE0EF Exit     \uE0E1 Back", ALIGNED_RIGHT);
    Gui::endDraw();
    return;
  } else if (!m_attached) {
    Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2 - 50, currTheme.textColor, "EdiZon couldn't attach to the running Application. Please restart \n EdiZon and try again.", ALIGNED_CENTER);
    Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 50, Gui::g_framebuffer_height - 50, currTheme.textColor, "\uE0EF Exit     \uE0E1 Back", ALIGNED_RIGHT);
    Gui::endDraw();
    return;
  }

  if (m_searchMode == SEARCH_BEGIN)
    Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 50, Gui::g_framebuffer_height - 50, currTheme.textColor, "\uE0EF Exit     \uE0F0 Frozen addresses     \uE0E3 Search RAM     \uE0E1 Back", ALIGNED_RIGHT);
  else if (m_searchMode == SEARCH_CONTINUE) {
    if (m_foundAddresses.size() > 0) {
      if (m_sysmodulePresent)
        Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 50, Gui::g_framebuffer_height - 50, currTheme.textColor, "\uE0EF Exit     \uE0F0 Reset search     \uE0E3 Search again     \uE0E2 Freeze value     \uE0E0 Edit value     \uE0E1 Back", ALIGNED_RIGHT);
      else 
        Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 50, Gui::g_framebuffer_height - 50, currTheme.textColor, "\uE0EF Exit     \uE0F0 Reset search     \uE0E3 Search again     \uE0E0 Edit value     \uE0E1 Back", ALIGNED_RIGHT);
    }
    else 
      Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 50, Gui::g_framebuffer_height - 50, currTheme.textColor, "\uE0EF Exit     \uE0F0 Reset search     \uE0E1 Back", ALIGNED_RIGHT);
  }

  Gui::drawRectangle(256, 50, Gui::g_framebuffer_width - 256, 206, currTheme.separatorColor);

  if (Title::g_titles[m_debugger->getRunningApplicationTID()] != nullptr)
    Gui::drawImage(0, 0, 256, 256, Title::g_titles[m_debugger->getRunningApplicationTID()]->getTitleIcon(), IMAGE_MODE_RGB24);
  else 
    Gui::drawRectangle(0, 0, 256, 256, Gui::makeColor(0x00, 0x00, 0xFF, 0xFF));

  Gui::drawRectangle(650, 65, 20, 20,  Gui::makeColor(0xFF, 0x00, 0x00, 0xFF)); // Code
  Gui::drawRectangle(650, 85, 20, 20,  Gui::makeColor(0x00, 0xFF, 0x00, 0xFF)); // Shared Memory
  Gui::drawRectangle(650, 105, 20, 20,  Gui::makeColor(0x00, 0x00, 0xFF, 0xFF)); // Heap
  Gui::drawRectangle(650, 125, 20, 20, Gui::makeColor(0xFF, 0xFF, 0x00, 0xFF)); // Stack
  Gui::drawRectangle(650, 145, 20, 20, Gui::makeColor(0x80, 0x80, 0x80, 0xFF)); // Others

  Gui::drawTextAligned(font14, 690, 62,  currTheme.textColor, "Code", ALIGNED_LEFT);
  Gui::drawTextAligned(font14, 690, 82,  currTheme.textColor, "Shared Memory", ALIGNED_LEFT);
  Gui::drawTextAligned(font14, 690, 102, currTheme.textColor, "Heap", ALIGNED_LEFT);
  Gui::drawTextAligned(font14, 690, 122, currTheme.textColor, "Stack", ALIGNED_LEFT);
  Gui::drawTextAligned(font14, 690, 142, currTheme.textColor, "Others", ALIGNED_LEFT);


  ss.str("");
  ss << "BASE  :  0x" <<std::uppercase << std::setfill('0') << std::setw(16) << std::hex << m_addressSpaceBaseAddr;
  Gui::drawTextAligned(font14, 900, 75,  currTheme.textColor, ss.str().c_str(), ALIGNED_LEFT);
  ss.str("");
  ss << "HEAP  :  0x" <<std::uppercase << std::setfill('0') << std::setw(16) << std::hex << m_heapBaseAddr;
  Gui::drawTextAligned(font14, 900, 105,  currTheme.textColor, ss.str().c_str(), ALIGNED_LEFT);
  ss.str("");
  ss << "MAIN  :  0x" <<std::uppercase << std::setfill('0') << std::setw(16) << std::hex << m_codeBaseAddr;
  Gui::drawTextAligned(font14, 900, 135, currTheme.textColor, ss.str().c_str(), ALIGNED_LEFT);


  Gui::drawRectangle(256, 50, 394, 137, COLOR_WHITE);

  Gui::drawTextAligned(font20, 280, 70, COLOR_BLACK, titleNameStr.c_str(), ALIGNED_LEFT);
  Gui::drawTextAligned(font14, 280, 120, COLOR_BLACK, tidStr.c_str(), ALIGNED_LEFT);
  Gui::drawTextAligned(font14, 280, 140, COLOR_BLACK, pidStr.c_str(), ALIGNED_LEFT);


  Gui::drawRectangle(256, 186, 92, 70, currTheme.selectedColor);
  Gui::drawRectangle(Gui::g_framebuffer_width - 92, 186, 92, 70, currTheme.selectedColor);

  for (u8 i = 0; i < 12; i++) {
    Gui::drawRectangle(348 + i * 70, 186, 70, 70, currTheme.textColor);
    Gui::drawRectangle(350 + i * 70, 186 + 2, 70 - 4, 70 - 4, m_searchType == i ? m_searchMode == SEARCH_BEGIN ? currTheme.highlightColor : currTheme.selectedColor : currTheme.separatorColor);
    Gui::drawTextAligned(font20, 385 + i * 70, 203, m_searchType == i ? COLOR_BLACK : currTheme.textColor, dataTypes[i].c_str(), ALIGNED_CENTER);
    
    if (i >= 8)
      Gui::drawRectangled(350 + i * 70, 186 + 2, 70 - 4, 70 - 4, Gui::makeColor(0x80, 0x80, 0x80, 0x80));
  }

  Gui::drawTextAligned(font20, 290, 203, COLOR_BLACK, "\uE0E4", ALIGNED_LEFT);
  Gui::drawTextAligned(font20, 1222, 203, COLOR_BLACK, "\uE0E5", ALIGNED_LEFT);


  if (m_cheatCnt > 0 && m_sysmodulePresent) {
    Gui::drawRectangle(50, 256, 650, 46 + std::min(static_cast<u32>(m_cheatCnt), 8U) * 40, currTheme.textColor);
    Gui::drawTextAligned(font14, 375, 262, currTheme.backgroundColor, "Cheats", ALIGNED_CENTER);
    Gui::drawShadow(50, 256, 650, 46 + std::min(static_cast<u32>(m_cheatCnt), 8U) * 40);

    for (u8 line = 0; line < 8; line++) {
      if (line >= m_cheatCnt) break;

      ss.str("");
      if (line < 7 && m_cheatCnt != 8) 
        ss << "\uE070   " << m_cheats[line].definition.readable_name;
      else 
        ss << "And " << std::dec << (m_cheatCnt - 8) << " more...";

      Gui::drawRectangle(52, 300 + line * 40, 646, 40, (m_selectedEntry == line && m_menuLocation == CHEATS) ? currTheme.highlightColor : line % 2 == 0 ? currTheme.backgroundColor : currTheme.separatorColor);
      Gui::drawTextAligned(font14, 70, 305 + line * 40, (m_selectedEntry == line && m_menuLocation == CHEATS) ? COLOR_BLACK : currTheme.textColor, ss.str().c_str(), ALIGNED_LEFT);
      
      if (!m_cheats[line].enabled) {
        color_t highlightColor = currTheme.highlightColor;
        highlightColor.a = 0xFF;

        Gui::drawRectangled(74, 313 + line * 40, 10, 10, (m_selectedEntry == line && m_menuLocation == CHEATS) ? highlightColor : line % 2 == 0 ? currTheme.backgroundColor : currTheme.separatorColor);
      }
    }
  }


  if (!m_foundAddresses.empty()) {
    Gui::drawRectangle(Gui::g_framebuffer_width - 552, 256, 500, 46 + std::min(static_cast<u32>(m_foundAddresses.size()), 8U) * 40, currTheme.textColor);
    Gui::drawTextAligned(font14, Gui::g_framebuffer_width - 302, 262, currTheme.backgroundColor, "Found candidates", ALIGNED_CENTER);
    Gui::drawShadow(Gui::g_framebuffer_width - 552, 256, 500, 46 + std::min(static_cast<u32>(m_foundAddresses.size()), 8U) * 40);
  }

  for (u8 line = 0; line < 8; line++) {
    if (line >= m_foundAddresses.size()) break;

    ss.str("");

    if (line < 7 && m_foundAddresses.size() != 8) {
      ss << getAddressDisplayString(m_foundAddresses[line], m_debugger, (GuiRAMEditor::searchType_t)m_searchType, m_heapBaseAddr, m_codeBaseAddr, m_addressSpaceBaseAddr);

    if (m_frozenAddresses.find(m_foundAddresses[line].addr) != m_frozenAddresses.end())
      ss << "   \uE130";
    }
    else 
      ss << "And " << std::dec << (m_foundAddresses.size() - 8) << " others...";

    Gui::drawRectangle(Gui::g_framebuffer_width - 550, 300 + line * 40, 496, 40, (m_selectedEntry == line && m_menuLocation == CANDIDATES) ? currTheme.highlightColor : line % 2 == 0 ? currTheme.backgroundColor : currTheme.separatorColor);
    Gui::drawTextAligned(font14, Gui::g_framebuffer_width - 530, 305 + line * 40, (m_selectedEntry == line && m_menuLocation == CANDIDATES) ? COLOR_BLACK : currTheme.textColor, ss.str().c_str(), ALIGNED_LEFT);
  }

  Gui::drawShadow(0, 0, Gui::g_framebuffer_width, 256);
  Gui::drawShadow(256, 50, Gui::g_framebuffer_width, 136);

  for (u16 x = 0; x < 1024; x++)
    Gui::drawRectangle(256 + x, 0, 1, 50, m_memory[x]);


  Gui::endDraw();
}

void GuiRAMEditor::onInput(u32 kdown) {
  if (kdown & KEY_B) 
    Gui::g_nextGui = GUI_MAIN;

  if (m_debugger->getRunningApplicationPID() == 0)
    return;

  if (kdown & KEY_UP)
    if (m_selectedEntry > 0)
      m_selectedEntry--;
  
  if (kdown & KEY_DOWN) {
    if (m_menuLocation == CANDIDATES) {
      if (m_selectedEntry < 7 && m_selectedEntry < (m_foundAddresses.size() - 1))
        m_selectedEntry++;
    } else {
      if (m_selectedEntry < 7 && m_selectedEntry < (m_cheatCnt - 1))
        m_selectedEntry++;
    }
  }


  if (m_searchMode == SEARCH_BEGIN) {
    if (kdown & KEY_L)
      if (m_searchType > 0)
        m_searchType--;

    if (kdown & KEY_R)
      if (m_searchType < 7)
        m_searchType++;
  }

  if (m_foundAddresses.size() > 0) {
    if (kdown & KEY_LEFT)
      if (m_cheatCnt > 0) {
        m_menuLocation = CHEATS;
        m_selectedEntry = 0;
      }

    if (kdown & KEY_RIGHT) {
      m_menuLocation = CANDIDATES;
      m_selectedEntry = 0;
    }
  }


  if (m_menuLocation == CANDIDATES) { /* Candidates menu */
    if (m_foundAddresses.size() == 0) return;
    if (m_searchMode == SEARCH_CONTINUE) { 
      if (kdown & KEY_X && m_sysmodulePresent) {
        if (!isAddressFrozen(m_foundAddresses[m_selectedEntry].addr)) {
          u64 outValue;

          if (R_SUCCEEDED(dmntchtEnableFrozenAddress(m_foundAddresses[m_selectedEntry].addr, dataTypeSizes[m_searchType], &outValue))) {
            (new Snackbar("Froze variable!"))->show();
            m_frozenAddresses.insert(m_foundAddresses[m_selectedEntry].addr);
          }
          else
            (new Snackbar("Failed to freeze variable!"))->show();
        }
        else {
          if (R_SUCCEEDED(dmntchtDisableFrozenAddress(m_foundAddresses[m_selectedEntry].addr))) {
            (new Snackbar("Unfroze variable!"))->show();
            m_frozenAddresses.erase(m_foundAddresses[m_selectedEntry].addr);
          }
          else
            (new Snackbar("Failed to unfreeze variable!"))->show();
        }
      }

      if (kdown & KEY_A) {
        if (m_selectedEntry < 7) {
          char input[16];
          if (Gui::requestKeyboardInput("Enter value", "Enter a value that should get written at this address.", "", SwkbdType::SwkbdType_NumPad, input, 15)) {
            m_debugger->pokeMemory(dataTypeSizes[m_searchType], m_foundAddresses[m_selectedEntry].addr, atol(input));
          }
        } else if (m_foundAddresses.size() < 25) {
          std::vector<std::string> options;
          options.clear();

          std::stringstream ss;
          for (u32 i = 8; i < m_foundAddresses.size(); i++) {
            ss.str("");
            ss << "0x" << std::uppercase << std::hex << std::setfill('0') << std::setw(10) << m_foundAddresses[i].addr;

            float fcast = 0.0F;
            double dcast = 0.0;
            u64 fvalue = 0;
            u64 dvalue = 0;
            
            switch(m_searchType) {
              case UNSIGNED_8BIT:
                ss << " (" << std::dec << static_cast<u32>(m_debugger->peekMemory(m_foundAddresses[i].addr) & 0xFF) << ")";
                break;
              case UNSIGNED_16BIT:
                ss << " (" << std::dec << static_cast<u32>(m_debugger->peekMemory(m_foundAddresses[i].addr) & 0xFFFF) << ")";
                break;
              case UNSIGNED_32BIT:
                ss << " (" << std::dec << static_cast<u32>(m_debugger->peekMemory(m_foundAddresses[i].addr)) << ")";
                break;
              case UNSIGNED_64BIT:
                ss << " (" << std::dec << static_cast<u64>(m_debugger->peekMemory(m_foundAddresses[i].addr)) << ")";
                break;
              case SIGNED_8BIT:
                ss << " (" << std::dec << static_cast<s32>(m_debugger->peekMemory(m_foundAddresses[i].addr) & 0xFF) << ")";
                break;
              case SIGNED_16BIT:
                ss << " (" << std::dec << static_cast<s32>(m_debugger->peekMemory(m_foundAddresses[i].addr) & 0xFFFF) << ")";
                break;
              case SIGNED_32BIT:
                ss << " (" << std::dec << static_cast<s32>(m_debugger->peekMemory(m_foundAddresses[i].addr)) << ")";
                break;
              case SIGNED_64BIT:
                ss << " (" << std::dec << static_cast<s64>(m_debugger->peekMemory(m_foundAddresses[i].addr)) << ")";
                break;
              case FLOAT_32BIT:
                fvalue = m_debugger->peekMemory(m_foundAddresses[i].addr);
                memcpy(&fcast, &fvalue, 4);
                ss << " (" << fcast << ")";
                break;
              case FLOAT_64BIT:
                dvalue = m_debugger->peekMemory(m_foundAddresses[i].addr);
                memcpy(&dcast, &dvalue, 8);
                ss << " (" << dcast << ")"; 
                break;
            }

            options.push_back(ss.str());
          }

          (new ListSelector("Edit value at address", "\uE0E0 Edit value     \uE0E1 Back", options))->setInputAction([&](u32 k, u16 selectedItem) {
            if (k & KEY_A) {
              char input[16];
              if (Gui::requestKeyboardInput("Enter value", "Enter a value for which the game's memory should be searched.", "", SwkbdType::SwkbdType_NumPad, input, 15)) {
                u64 value = atol(input);
                if (value > dataTypeMaxValues[m_searchType] || value < dataTypeMinValues[m_searchType]) {
                  (new Snackbar("Entered value isn't inside the range of this data type. Please enter a different value."))->show();
                  return;
                }

                m_debugger->pokeMemory(dataTypeSizes[m_searchType], m_foundAddresses[m_selectedEntry].addr, value);
              }
            }
          })->show();
        } else (new Snackbar("Too many addresses! Try narrowing down the selection a bit before editing."))->show();
      }
    }
  } else { /* Cheats menu */
    if (kdown & KEY_A) {
      if (m_cheatCnt == 0) return;

      if (m_selectedEntry != 7) {
        dmntchtToggleCheat(m_cheats[m_selectedEntry].cheat_id);
        u64 cheatCnt = 0;

        dmntchtGetCheatCount(&cheatCnt);
        if (cheatCnt > 0) {
          delete[] m_cheats;
          m_cheats = new DmntCheatEntry[cheatCnt];
          dmntchtGetCheats(m_cheats, cheatCnt, 0, &m_cheatCnt);
        }
      } else {
        std::vector<std::string> options;

        for (u16 i = 7; i < m_cheatCnt; i++)
          options.push_back((m_cheats[i].enabled ? "\uE070    " : "        ") + std::string(m_cheats[i].definition.readable_name));

        (new ListSelector("Cheats", "\uE0E0 Toggle cheat     \uE0E1 Back", options))->setInputAction([&](u32 k, u16 selectedItem) {
          if (k & KEY_A) {
            dmntchtToggleCheat(m_cheats[selectedItem + 7].cheat_id);
            u64 cheatCnt = 0;

            dmntchtGetCheatCount(&cheatCnt);
            if (cheatCnt > 0) {
              delete[] m_cheats;
              m_cheats = new DmntCheatEntry[cheatCnt];
              dmntchtGetCheats(m_cheats, cheatCnt, 0, &m_cheatCnt);
            }

            Gui::g_currListSelector->hide();
          }
        })->show();
      }
    }
  }

  if (kdown & KEY_MINUS) {
    if (m_foundAddresses.size() == 0) {
      std::vector<std::string> options;
      for (u64 addr : m_frozenAddresses)
        options.push_back(getAddressDisplayString({ addr, MemType_Normal }, m_debugger, (GuiRAMEditor::searchType_t)m_searchType, m_heapBaseAddr, m_codeBaseAddr, m_addressSpaceBaseAddr));

      if (m_frozenAddresses.size() == 0) {
        (new Snackbar("No frozen variables were found."))->show();
        return;
      }

      (new ListSelector("Frozen Addresses", "\uE0E0 Unfreeze     \uE0E1 Back", options))->setInputAction([&](u32 k, u16 selectedItem) {
        if (k & KEY_A) {
          auto itr = m_frozenAddresses.begin();
          std::advance(itr, selectedItem);
          
          dmntchtDisableFrozenAddress(*itr);
          m_frozenAddresses.erase(*itr);

          Gui::g_currListSelector->hide();
        }
      })->show();
    } else {
      m_searchMode = SEARCH_BEGIN;
      m_foundAddresses.clear();
      m_menuLocation = CHEATS;
      remove("/EdiZon/addresses.dat");
    }
  }

  if (kdown & KEY_Y) {
    char input[16];
    bool entered = false;
    s128 searchValue = 0;
    float floatValue = 0.0F;
    double doubleValue = 0.0;

    switch(m_searchType) {
      case SIGNED_8BIT:  [[fallthrough]]
      case SIGNED_16BIT: [[fallthrough]]
      case SIGNED_32BIT: [[fallthrough]]
      case SIGNED_64BIT: 
        entered = Gui::requestKeyboardInput("Enter value", "Enter a value for which the game's memory should be searched.", "", SwkbdType::SwkbdType_NumPad, input, 15);
        searchValue = -atol(input);
        break;
      case UNSIGNED_8BIT:  [[fallthrough]]
      case UNSIGNED_16BIT: [[fallthrough]]
      case UNSIGNED_32BIT: [[fallthrough]]
      case UNSIGNED_64BIT: 
        entered = Gui::requestKeyboardInput("Enter value", "Enter a value for which's negated value the game's memory should be searched. (50 -> -50)", "", SwkbdType::SwkbdType_NumPad, input, 15);
        searchValue = atol(input);
        break;
      case FLOAT_32BIT:
        entered = Gui::requestKeyboardInput("Enter value", "Enter a floating point value for which's negated value the game's memory should be searched. (Only use numbers, dots and dashes)", "", SwkbdType::SwkbdType_QWERTY, input, 15);
        floatValue = std::stof(input);
        searchValue = (s128)(*(s32*)&floatValue);
        break;
      case FLOAT_64BIT:
        entered = Gui::requestKeyboardInput("Enter value", "Enter a floating point value for which's negated value the game's memory should be searched. (Only use numbers, dots and dashes)", "", SwkbdType::SwkbdType_QWERTY, input, 15);
        doubleValue = std::stod(input);
        searchValue = (s128)(*(s64*)&doubleValue);
        break;
      case STRING:  [[fallthrough]]
      case POINTER:
        (new Snackbar("Not supported yet!"))->show();
        break;
    }

    if (entered) {
      if (searchValue > dataTypeMaxValues[m_searchType] || searchValue < dataTypeMinValues[m_searchType]) {
        (new Snackbar("Entered value isn't inside the range of this data type. Please choose a different one."))->show();
        return;
      }

      m_debugger->breakProcess();

      (new MessageBox("Searching RAM \n \n This may take a while...", MessageBox::NONE))->show();
      requestDraw();

      if (m_searchMode == SEARCH_CONTINUE) {
        m_selectedEntry = 0;

        std::vector<GuiRAMEditor::ramAddr_t> newAddresses;
        for (GuiRAMEditor::ramAddr_t addr : m_foundAddresses) {
          u64 value = 0, realValue = 0;

          m_debugger->readMemory(&value, 8, addr.addr);
          memcpy(&realValue, &value, dataTypeSizes[m_searchType]);

          if (realValue == searchValue) {
            newAddresses.push_back(addr);
          }
        }

        m_foundAddresses.clear();
        std::copy(newAddresses.begin(), newAddresses.end(), std::back_inserter(m_foundAddresses));

        if (m_foundAddresses.empty()) {
          m_searchMode = SEARCH_BEGIN;
          remove("/EdiZon/addresses.dat");
          
          Gui::g_currMessageBox->hide();
          m_debugger->continueProcess();
          (new Snackbar("None of your previously found addresses got changed to the entered value."))->show();
          return;
        }

      } else {
        m_searchMode = SEARCH_CONTINUE;
        m_selectedEntry = 0;

        for (MemoryInfo meminfo : m_memoryInfo) {
          if (m_searchType != POINTER && meminfo.type != MemType_Heap) continue;
          u64 offset = 0;
          u64 bufferSize = 0x10000;
          u8 *buffer = new u8[bufferSize];
          while (offset < meminfo.size) {
            
            if (meminfo.size - offset < bufferSize)
              bufferSize = meminfo.size - offset;

            m_debugger->readMemory(buffer, bufferSize, meminfo.addr + offset);

            u64 realValue = 0;
            for (u64 i = 0; i < bufferSize; i++) {
              memcpy(&realValue, buffer + i, dataTypeSizes[m_searchType]);

              if (realValue == searchValue) {
                m_foundAddresses.push_back({ .addr = meminfo.addr + offset + i, .type = (MemoryType) meminfo.type });
              }
            }

            offset += bufferSize;
          }

          delete[] buffer;
        }

        if (m_foundAddresses.empty()) {
          m_searchMode = SEARCH_BEGIN;
          remove("/EdiZon/addresses.dat");
          (new Snackbar("Value not found in memory. Try again with a different one."))->show();
        }
      }

      std::ofstream file("/EdiZon/addresses.dat", std::ios::out | std::ios::binary);
      if (file.is_open()) {
        u64 addressCount = m_foundAddresses.size();
        file.write((char*)&addressCount, 8);
        file.write((char*)&m_searchType, 1);
        file.write((char*)&m_heapBaseAddr, 8);
        file.write((char*)&m_foundAddresses[0], m_foundAddresses.size() * sizeof(GuiRAMEditor::ramAddr_t));
        file.close();
      } else printf("Didn't save!\n");

      Gui::g_currMessageBox->hide();

      m_debugger->continueProcess();

    }
  }
}

void GuiRAMEditor::onTouch(touchPosition &touch) {

}

void GuiRAMEditor::onGesture(touchPosition startPosition, touchPosition endPosition, bool finish) {

}

bool isAddressFrozen(uintptr_t address) {
  DmntFrozenAddressEntry *addresses;
  u64 addressCnt = 0;
  bool frozen = false;

  dmntchtGetFrozenAddressCount(&addressCnt);

  if (addressCnt != 0) {
    addresses = new DmntFrozenAddressEntry[addressCnt];
    dmntchtGetFrozenAddresses(addresses, addressCnt, 0, nullptr);

    for (u64 i = 0; i < addressCnt; i++) {
      if (addresses[i].address == address) {
        frozen = true;
        break;
      }
    }
  }

  return frozen;
}

std::string getAddressDisplayString(GuiRAMEditor::ramAddr_t address, Debugger *debugger, GuiRAMEditor::searchType_t searchType, u64 heapBaseAddr, u64 codeBaseAddr, u64 addressSpaceBaseAddr) {
  std::stringstream ss;

  if (address.type == MemType_Heap)
    ss << "[ HEAP + 0x" << std::uppercase << std::hex << std::setfill('0') << std::setw(10) << (address.addr - heapBaseAddr) << " ]";
  else if (address.type == MemType_CodeStatic || address.type == MemType_CodeMutable)
    ss << "[ MAIN + 0x" << std::uppercase << std::hex << std::setfill('0') << std::setw(10) << (address.addr - codeBaseAddr) << " ]";
  else
    ss << "[ BASE + 0x" << std::uppercase << std::hex << std::setfill('0') << std::setw(10) << (address.addr - addressSpaceBaseAddr) << " ]";

  float fcast = 0.0F;
  double dcast = 0.0;
  u64 fvalue = 0;
  u64 dvalue = 0;

  switch(searchType) {
    case GuiRAMEditor::UNSIGNED_8BIT:
      ss << "  ( " << std::dec << static_cast<u32>(debugger->peekMemory(address.addr) & 0xFF) << " )";
      break;
    case GuiRAMEditor::UNSIGNED_16BIT:
      ss << "  ( " << std::dec << static_cast<u32>(debugger->peekMemory(address.addr) & 0xFFFF) << " )";
      break;
    case GuiRAMEditor::UNSIGNED_32BIT:
      ss << "  ( " << std::dec << static_cast<u32>(debugger->peekMemory(address.addr)) << " )";
      break;
    case GuiRAMEditor::UNSIGNED_64BIT:
      ss << "  ( " << std::dec << static_cast<u64>(debugger->peekMemory(address.addr)) << " )";
      break;
    case GuiRAMEditor::SIGNED_8BIT:
      ss << "  ( " << std::dec << static_cast<s32>(debugger->peekMemory(address.addr) & 0xFF) << " )";
      break;
    case GuiRAMEditor::SIGNED_16BIT:
      ss << "  ( " << std::dec << static_cast<s32>(debugger->peekMemory(address.addr) & 0xFFFF) << " )";
      break;
    case GuiRAMEditor::SIGNED_32BIT:
      ss << "  ( " << std::dec << static_cast<s32>(debugger->peekMemory(address.addr)) << " )";
      break;
    case GuiRAMEditor::SIGNED_64BIT:
      ss << "  ( " << std::dec << static_cast<s64>(debugger->peekMemory(address.addr)) << " )";
      break;
    case GuiRAMEditor::FLOAT_32BIT:
      fvalue = debugger->peekMemory(address.addr);
      memcpy(&fcast, &fvalue, 4);
      ss << "  ( " << fcast << " )";
      break;
    case GuiRAMEditor::FLOAT_64BIT:
      dvalue = debugger->peekMemory(address.addr);
      memcpy(&dcast, &dvalue, 8);
      ss << "  ( " << dcast << " )"; 
      break;
    case GuiRAMEditor::POINTER: break;
    case GuiRAMEditor::STRING: break;
    default: break;
  }

  return ss.str();
}