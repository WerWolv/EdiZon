#include "guis/gui_cheats.hpp"

#include <sstream>
#include <limits>
#include <utility>

#include <bits/stdc++.h>
#include <thread>

extern "C" {
  #include "helpers/util.h"
}

#include "edizon_logo_bin.h"


static const std::vector<std::string> dataTypes = { "u8", "s8", "u16", "s16", "u32", "s32", "u64", "s64", "f32", "f64", "ptr" };
static const std::vector<u8> dataTypeSizes      = {    1,   1,     2,     2,     4,     4,     8,     8,     4,     8,     8 };
static const std::vector<s128> dataTypeMaxValues = { std::numeric_limits<u8>::max(), std::numeric_limits<s8>::max(), std::numeric_limits<u16>::max(), std::numeric_limits<s16>::max(), std::numeric_limits<u32>::max(), std::numeric_limits<s32>::max(), std::numeric_limits<u64>::max(), std::numeric_limits<s64>::max(), std::numeric_limits<s32>::max(), std::numeric_limits<s64>::max(), std::numeric_limits<u64>::max() };
static const std::vector<s128> dataTypeMinValues = { std::numeric_limits<u8>::min(), std::numeric_limits<s8>::min(), std::numeric_limits<u16>::min(), std::numeric_limits<s16>::min(), std::numeric_limits<u32>::min(), std::numeric_limits<s32>::min(), std::numeric_limits<u64>::min(), std::numeric_limits<s64>::min(), std::numeric_limits<s32>::min(), std::numeric_limits<s64>::min(), std::numeric_limits<u64>::min() };

static std::string titleNameStr, tidStr, pidStr, buildIDStr;

static u32 cheatListOffset = 0;

bool _isAddressFrozen(uintptr_t );
std::string _getAddressDisplayString(u64 , Debugger *debugger, searchType_t searchType);
std::string _getValueDisplayString(searchValue_t searchValue, searchType_t searchType);

GuiRAMEditor::GuiRAMEditor() : Gui() {
  
  m_sysmodulePresent = isServiceRunning("dmnt:cht");
  m_debugger = new Debugger(m_sysmodulePresent);
  m_cheats = nullptr;
  m_foundAddresses = nullptr;

  if (m_debugger->getRunningApplicationPID() == 0) {
    if (!remove("/switch/EdiZon/memdump.dat"))
      Gui::g_nextGui = GUI_MAIN;
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
    memcpy(m_buildID, metadata.main_nso_build_id, 0x20);

    u64 cheatCnt = 0;

    dmntchtGetCheatCount(&cheatCnt);

    if (cheatCnt > 0) {
      m_cheats = new DmntCheatEntry[cheatCnt];
      dmntchtGetCheats(m_cheats, cheatCnt, 0, &m_cheatCnt);
    }

    u64 Cnt = 0;
    dmntchtGetFrozenAddressCount(&Cnt);

    if (Cnt != 0) {
      DmntFrozenAddressEntry frozenAddresses[Cnt];
      dmntchtGetFrozenAddresses(frozenAddresses, Cnt, 0, nullptr);

      for (u16 i = 0; i < Cnt; i++)
        m_frozenAddresses.insert({ frozenAddresses[i].address, frozenAddresses[i].value.value });

    }
  } else
    return;

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

  bool memdumpFileExists = access("/switch/EdiZon/memdump.dat", F_OK) == 0;

  m_foundAddresses = new MemoryDump("/switch/EdiZon/memdump.dat", m_heapBaseAddr, m_searchValue, m_searchType, m_searchRegion, !memdumpFileExists);

  if (memdumpFileExists) {
    m_searchType = m_foundAddresses->getSearchType();
    m_searchValue = m_foundAddresses->getSearchValue();
    m_searchRegion = m_foundAddresses->getSearchRegion();
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
  ss.str("");

  ss << "BID: ";
  for (u8 i = 0; i < 8; i++)
    ss << std::nouppercase << std::hex << std::setfill('0') << std::setw(2) << (u16)m_buildID[i];
  
  buildIDStr = ss.str();

  if (!m_sysmodulePresent) {
    (new MessageBox("Atmosphère's cheat module is not running on this System. \n Cheat management and variable freezing is disabled.", MessageBox::OKAY))->show();
  }

  if (m_cheatCnt == 0)
    m_menuLocation = CANDIDATES;
  if (m_foundAddresses->size() == 0)
    m_menuLocation = CHEATS;

  appletSetMediaPlaybackState(true);
}

GuiRAMEditor::~GuiRAMEditor() {
  m_debugger->detachFromProcess();

  delete m_debugger;

  if (m_foundAddresses != nullptr)
    delete m_foundAddresses;

  if (m_cheats != nullptr)
    delete[] m_cheats;

  if (m_sysmodulePresent) {
    amspmdmntExit();
    dmntchtExit();
  }

  appletSetMediaPlaybackState(false);
}



void GuiRAMEditor::update() {
  Gui::update();
}

void GuiRAMEditor::draw() {
  static u32 splashCnt = 0;
  std::stringstream ss;

  Gui::beginDraw();

  if (!Gui::g_splashDisplayed) {
    Gui::drawRectangle(0, 0, Gui::g_framebuffer_width, Gui::g_framebuffer_height, Gui::makeColor(0x5D, 0x4F, 0x4E, 0xFF));
    Gui::drawImage(Gui::g_framebuffer_width / 2 - 128, Gui::g_framebuffer_height / 2 - 128, 256, 256, edizon_logo_bin, IMAGE_MODE_BGR24);

    if (splashCnt++ >= 70)
      Gui::g_splashDisplayed = true;

    Gui::endDraw();
    return;
  }

  Gui::drawRectangle(0, 0, Gui::g_framebuffer_width, Gui::g_framebuffer_height, currTheme.backgroundColor);

  Gui::drawRectangle((u32)((Gui::g_framebuffer_width - 1220) / 2), Gui::g_framebuffer_height - 73, 1220, 1, currTheme.textColor);


  if (m_debugger->getRunningApplicationPID() == 0) {
    Gui::drawTextAligned(fontHuge, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2 - 100, currTheme.textColor, "\uE12C", ALIGNED_CENTER);
    Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2, currTheme.textColor, "A title needs to be running in the background to use the RAM editor. \n Please launch an application and try again.", ALIGNED_CENTER);
    Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 50, Gui::g_framebuffer_height - 50, currTheme.textColor, "\uE0E1 Back", ALIGNED_RIGHT);
    Gui::endDraw();
    return;
  } else if (!m_attached) {
    Gui::drawTextAligned(fontHuge, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2 - 100, currTheme.textColor, "\uE142", ALIGNED_CENTER);
    Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2, currTheme.textColor, "EdiZon couldn't attach to the running Application. Please restart \n EdiZon and try again.", ALIGNED_CENTER);
    Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 50, Gui::g_framebuffer_height - 50, currTheme.textColor, "\uE0E1 Back", ALIGNED_RIGHT);
    Gui::endDraw();
    return;
  }

  if (m_foundAddresses->size() == 0) {
    if (m_frozenAddresses.size() != 0)
      Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 50, Gui::g_framebuffer_height - 50, currTheme.textColor, "\uE0F0 Frozen es     \uE0E3 Search RAM     \uE0E1 Back", ALIGNED_RIGHT);
    else
      Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 50, Gui::g_framebuffer_height - 50, currTheme.textColor, "\uE0E3 Search RAM     \uE0E1 Back", ALIGNED_RIGHT);
  } else {
    if (m_foundAddresses->size() > 0) {
      if (m_sysmodulePresent)
        Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 50, Gui::g_framebuffer_height - 50, currTheme.textColor, "\uE0F0 Reset search     \uE0E3 Search again     \uE0E2 Freeze value     \uE0E0 Edit value     \uE0E1 Back", ALIGNED_RIGHT);
      else 
        Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 50, Gui::g_framebuffer_height - 50, currTheme.textColor, "\uE0F0 Reset search     \uE0E3 Search again     \uE0E0 Edit value     \uE0E1 Back", ALIGNED_RIGHT);
    }
    else 
      Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 50, Gui::g_framebuffer_height - 50, currTheme.textColor, "\uE0F0 Reset search     \uE0E1 Back", ALIGNED_RIGHT);
  }

  Gui::drawRectangle(256, 50, Gui::g_framebuffer_width - 256, 206, currTheme.separatorColor);

  if (Title::g_titles[m_debugger->getRunningApplicationTID()] != nullptr)
    Gui::drawImage(0, 0, 256, 256, Title::g_titles[m_debugger->getRunningApplicationTID()]->getTitleIcon(), IMAGE_MODE_RGB24);
  else 
    Gui::drawRectangle(0, 0, 256, 256, Gui::makeColor(0x00, 0x00, 0xFF, 0xFF));

  Gui::drawRectangle(660, 65, 20, 20,  Gui::makeColor(0xFF, 0x00, 0x00, 0xFF));  // Code
  Gui::drawRectangle(660, 85, 20, 20,  Gui::makeColor(0x00, 0xFF, 0x00, 0xFF));  // Shared Memory
  Gui::drawRectangle(660, 105, 20, 20, Gui::makeColor(0x00, 0x00, 0xFF, 0xFF));  // Heap
  Gui::drawRectangle(660, 125, 20, 20, Gui::makeColor(0xFF, 0xFF, 0x00, 0xFF));  // Stack
  Gui::drawRectangle(660, 145, 20, 20, Gui::makeColor(0x80, 0x80, 0x80, 0xFF));  // Others

  Gui::drawTextAligned(font14, 700, 62,  currTheme.textColor, "Code", ALIGNED_LEFT);
  Gui::drawTextAligned(font14, 700, 82,  currTheme.textColor, "Shared Memory", ALIGNED_LEFT);
  Gui::drawTextAligned(font14, 700, 102, currTheme.textColor, "Heap", ALIGNED_LEFT);
  Gui::drawTextAligned(font14, 700, 122, currTheme.textColor, "Stack", ALIGNED_LEFT);
  Gui::drawTextAligned(font14, 700, 142, currTheme.textColor, "Others", ALIGNED_LEFT);


  ss.str("");
  ss << "BASE  :  0x" << std::uppercase << std::setfill('0') << std::setw(16) << std::hex << m_addressSpaceBaseAddr;
  Gui::drawTextAligned(font14, 900, 75,  currTheme.textColor, ss.str().c_str(), ALIGNED_LEFT);
  ss.str("");
  ss << "HEAP  :  0x" << std::uppercase << std::setfill('0') << std::setw(16) << std::hex << m_heapBaseAddr;
  Gui::drawTextAligned(font14, 900, 105,  currTheme.textColor, ss.str().c_str(), ALIGNED_LEFT);
  ss.str("");
  ss << "MAIN  :  0x" << std::uppercase << std::setfill('0') << std::setw(16) << std::hex << m_codeBaseAddr;
  Gui::drawTextAligned(font14, 900, 135, currTheme.textColor, ss.str().c_str(), ALIGNED_LEFT);


  Gui::drawRectangle(256, 50, 394, 137, COLOR_WHITE);

  Gui::drawTextAligned(font20, 280, 70, COLOR_BLACK, titleNameStr.c_str(), ALIGNED_LEFT);
  Gui::drawTextAligned(font14, 290, 110, COLOR_BLACK, tidStr.c_str(), ALIGNED_LEFT);
  Gui::drawTextAligned(font14, 290, 130, COLOR_BLACK, pidStr.c_str(), ALIGNED_LEFT);
  Gui::drawTextAligned(font14, 290, 150, COLOR_BLACK, buildIDStr.c_str(), ALIGNED_LEFT);

  if (m_cheatCnt > 0 && m_sysmodulePresent) {
    Gui::drawRectangle(50, 256, 650, 46 + std::min(static_cast<u32>(m_cheatCnt), 8U) * 40, currTheme.textColor);
    Gui::drawTextAligned(font14, 375, 262, currTheme.backgroundColor, "Cheats", ALIGNED_CENTER);
    Gui::drawShadow(50, 256, 650, 46 + std::min(static_cast<u32>(m_cheatCnt), 8U) * 40);

    for (u8 line = cheatListOffset; line < 8 + cheatListOffset; line++) {
      if (line >= m_cheatCnt) break;

      ss.str("");
      ss << "\uE070   " << m_cheats[line].definition.readable_name;

      Gui::drawRectangle(52, 300 + (line - cheatListOffset) * 40, 646, 40, (m_selectedEntry == line && m_menuLocation == CHEATS) ? currTheme.highlightColor : line % 2 == 0 ? currTheme.backgroundColor : currTheme.separatorColor);
      Gui::drawTextAligned(font14, 70, 305 + (line - cheatListOffset) * 40, (m_selectedEntry == line && m_menuLocation == CHEATS) ? COLOR_BLACK : currTheme.textColor, ss.str().c_str(), ALIGNED_LEFT);
      
      if (!m_cheats[line].enabled) {
        color_t highlightColor = currTheme.highlightColor;
        highlightColor.a = 0xFF;

        Gui::drawRectangled(74, 313 + (line - cheatListOffset) * 40, 10, 10, (m_selectedEntry == line && m_menuLocation == CHEATS) ? highlightColor : line % 2 == 0 ? currTheme.backgroundColor : currTheme.separatorColor);
      }
    }
  }


  if (m_foundAddresses->size() != 0) {
    Gui::drawRectangle(Gui::g_framebuffer_width - 552, 256, 500, 46 + std::min(static_cast<u32>(m_foundAddresses->size()), 8U) * 40, currTheme.textColor);
    Gui::drawTextAligned(font14, Gui::g_framebuffer_width - 302, 262, currTheme.backgroundColor, "Found candidates", ALIGNED_CENTER);
    Gui::drawShadow(Gui::g_framebuffer_width - 552, 256, 500, 46 + std::min(static_cast<u32>(m_foundAddresses->size()), 8U) * 40);
  }

  for (u8 line = 0; line < 8; line++) {
    if (line >= m_foundAddresses->size()) break;

    ss.str("");

    if (line < 7 && m_foundAddresses->size() != 8) {
      if (m_foundAddresses->getAddress(line).type == MemType_Heap)
        ss << "[ HEAP + 0x" << std::uppercase << std::hex << std::setfill('0') << std::setw(10) << (m_foundAddresses->getAddress(line).addr - m_heapBaseAddr) << " ]";
      else if (m_foundAddresses->getAddress(line).type == MemType_CodeStatic || m_foundAddresses->getAddress(line).type == MemType_CodeMutable)
        ss << "[ MAIN + 0x" << std::uppercase << std::hex << std::setfill('0') << std::setw(10) << (m_foundAddresses->getAddress(line).addr - m_codeBaseAddr) << " ]";
      else
        ss << "[ BASE + 0x" << std::uppercase << std::hex << std::setfill('0') << std::setw(10) << (m_foundAddresses->getAddress(line).addr - m_addressSpaceBaseAddr) << " ]";

      ss << "  ( " << _getAddressDisplayString(m_foundAddresses->getAddress(line).addr, m_debugger, (searchType_t)m_searchType) << " )";

    if (m_frozenAddresses.find(m_foundAddresses->getAddress(line).addr) != m_frozenAddresses.end())
      ss << "   \uE130";
    }
    else 
      ss << "And " << std::dec << (m_foundAddresses->size() - 8) << " others...";

    Gui::drawRectangle(Gui::g_framebuffer_width - 550, 300 + line * 40, 496, 40, (m_selectedEntry == line && m_menuLocation == CANDIDATES) ? currTheme.highlightColor : line % 2 == 0 ? currTheme.backgroundColor : currTheme.separatorColor);
    Gui::drawTextAligned(font14, Gui::g_framebuffer_width - 530, 305 + line * 40, (m_selectedEntry == line && m_menuLocation == CANDIDATES) ? COLOR_BLACK : currTheme.textColor, ss.str().c_str(), ALIGNED_LEFT);
  }

  Gui::drawShadow(0, 0, Gui::g_framebuffer_width, 256);
  Gui::drawShadow(256, 50, Gui::g_framebuffer_width, 136);

  for (u16 x = 0; x < 1024; x++)
    Gui::drawRectangle(256 + x, 0, 1, 50, m_memory[x]);

  drawSearchRAMMenu();

  Gui::endDraw();
}

void GuiRAMEditor::drawSearchRAMMenu() {
  static u32 cursorBlinkCnt = 0;
  u32 strWidth = 0;
  std::stringstream ss;

  if (m_searchMenuLocation == SEARCH_NONE) return;

  Gui::drawRectangled(0, 0, Gui::g_framebuffer_width, Gui::g_framebuffer_height, Gui::makeColor(0x00, 0x00, 0x00, 0xA0));

  Gui::drawRectangle(50, 50, Gui::g_framebuffer_width - 100, Gui::g_framebuffer_height - 100, currTheme.backgroundColor);
  Gui::drawRectangle(100, 135, Gui::g_framebuffer_width - 200, 1, currTheme.textColor);
  Gui::drawText(font24, 120, 70, currTheme.textColor, "\uE132   Search Memory");

  Gui::drawTextAligned(font20, 100, 160, currTheme.textColor, "\uE149 \uE0A4", ALIGNED_LEFT);
  Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 100, 160, currTheme.textColor, "\uE0A5 \uE14A", ALIGNED_RIGHT);

  Gui::drawTextAligned(font20, 260, 160,  m_searchMenuLocation == SEARCH_TYPE   ? currTheme.selectedColor : currTheme.textColor, "TYPE",   ALIGNED_CENTER);
  Gui::drawTextAligned(font20, 510, 160,  m_searchMenuLocation == SEARCH_MODE   ? currTheme.selectedColor : currTheme.textColor, "MODE",   ALIGNED_CENTER);
  Gui::drawTextAligned(font20, 760, 160,  m_searchMenuLocation == SEARCH_REGION ? currTheme.selectedColor : currTheme.textColor, "REGION", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, 1010, 160, m_searchMenuLocation == SEARCH_VALUE  ? currTheme.selectedColor : currTheme.textColor, "VALUE",  ALIGNED_CENTER);

  static const char* const typeNames[]    = { "u8", "s8", "u16", "s16", "u32", "s32", "u64", "s64", "flt", "dbl", "void*" };
  static const char* const modeNames[]    = { "==", "!=", ">", ">=", "<", "<=", "A : B", "SAME", "DIFF", "+ +", "- -" };
  static const char* const regionNames[]  = { "HEAP", "MAIN", "HEAP + MAIN", "RAM" };

  switch (m_searchMenuLocation) {
    case SEARCH_TYPE:
      for (u8 i = 0; i < 11; i++) {
        if (m_selectedEntry == i)
          Gui::drawRectangled(356 + (i / 2) * 100, 220 + (i % 2) * 100, 90, 90, m_searchType == static_cast<searchType_t>(i) ? currTheme.selectedColor : currTheme.highlightColor);

        Gui::drawRectangled(361 + (i / 2) * 100, 225 + (i % 2) * 100, 80, 80, currTheme.separatorColor);
        Gui::drawTextAligned(font20, 400 + (i / 2) * 100, 250 + (i % 2) * 100, currTheme.textColor, typeNames[i], ALIGNED_CENTER);
      }

      Gui::drawTextAligned(font14, Gui::g_framebuffer_width / 2, 500, currTheme.textColor,  "Set the data type of the value you’re searching here. The prefix [u] means unsigned (positive integers), [s] means \n"
                                                                                            "signed (positive and negative integers), [flt] is for floating point numbers (rational numbers), [dbl] is for double (bigger \n"
                                                                                            "rational numbers) and [void*] stands for pointer (link to another memory ) which is useful for creating cheats. The \n"
                                                                                            "number that follows is the number of bits used in memory which determines the maximum value. Choose the data type that \n"
                                                                                            "best fits for the type of data you’re looking for.", ALIGNED_CENTER);

      Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 100, Gui::g_framebuffer_height - 100, currTheme.textColor, "\uE0E1 Back     \uE0E0 OK", ALIGNED_RIGHT);

      break;
    case SEARCH_MODE:
      for (u8 i = 0; i < 11; i++) {
        if (m_selectedEntry == i)
          Gui::drawRectangled(356 + (i / 2) * 100, 220 + (i % 2) * 100, 90, 90, m_searchMode == static_cast<searchMode_t>(i) ? currTheme.selectedColor : currTheme.highlightColor);

        Gui::drawRectangled(361 + (i / 2) * 100, 225 + (i % 2) * 100, 80, 80, currTheme.separatorColor);
        Gui::drawTextAligned(font20, 400 + (i / 2) * 100, 250 + (i % 2) * 100, currTheme.textColor, modeNames[i], ALIGNED_CENTER);
      }

      Gui::drawTextAligned(font14, Gui::g_framebuffer_width / 2, 500, currTheme.textColor,  "Set the mode you want to use for finding values. With these modes EdiZon will search for values that are equal to [==], \n"
                                                                                            "not equal to [!=], greater than [>], greater than or equal to [>=], less than [<], or less than or equal to [<=] the value \n"
                                                                                            "that you input. [A : B] allows you to set a (min : max) range of values, SAME and DIFF search allows you to find values that \n"
                                                                                            "stayed the same or changed since the last search, [+ +] and [- -] checks for values that increased or decreased since the \n"
                                                                                            "previous search.", ALIGNED_CENTER);
      
      Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 100, Gui::g_framebuffer_height - 100, currTheme.textColor, "\uE0E1 Back     \uE0E0 OK", ALIGNED_RIGHT);
      break;
    case SEARCH_REGION:
      for (u8 i = 0; i < 4; i++) {
        if (m_selectedEntry == i)
          Gui::drawRectangled((Gui::g_framebuffer_width / 2) - 155, 215 + i * 70, 310, 70, m_searchRegion == static_cast<searchRegion_t>(i) ? currTheme.selectedColor : currTheme.highlightColor);

        Gui::drawRectangled((Gui::g_framebuffer_width / 2) - 150, 220 + i * 70, 300, 60, currTheme.separatorColor);
        Gui::drawTextAligned(font20, (Gui::g_framebuffer_width / 2), 235 + i * 70, currTheme.textColor, regionNames[i], ALIGNED_CENTER);
      }


      Gui::drawTextAligned(font14, Gui::g_framebuffer_width / 2, 500, currTheme.textColor,  "Set the memory region you want to search in. HEAP contains dynamically allocated values and will be where the majority of \n"
                                                                                            "values worth changing will be found. MAIN contains global variables and instructions for game operation. You may find some \n"
                                                                                            "values here but it’s mainly for finding pointers to HEAP values or changing game code. RAM will search the entirety of the Games \n"
                                                                                            "used memory including memory shared memory and resources. Should only be used as a final resort as this will be extremely slow. \n", ALIGNED_CENTER);
      
      Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 100, Gui::g_framebuffer_height - 100, currTheme.textColor, "\uE0E1 Back     \uE0E0 OK", ALIGNED_RIGHT);
      break;
    case SEARCH_VALUE:
      Gui::drawTextAligned(font14, Gui::g_framebuffer_width / 2, 500, currTheme.textColor,  "Set the value you want to search for. The value(s) you enter here will depend on what options you've chosen in the \n"
                                                                                            "first three sections. Either it's the exact integer you want to search for, a floating point number or even two values that \n"
                                                                                            "will be used as range.", ALIGNED_CENTER);

      //Gui::drawRectangle(300, 250, Gui::g_framebuffer_width - 600, 80, currTheme.separatorColor);
      Gui::drawRectangle(300, 327, Gui::g_framebuffer_width - 600, 3, currTheme.textColor);
      if (m_searchValueFormat == FORMAT_DEC)
        ss << _getValueDisplayString(m_searchValue, m_searchType);
      else if (m_searchValueFormat == FORMAT_HEX)
        ss << "0x" << std::uppercase << std::hex << m_searchValue._u64;

      Gui::getTextDimensions(font20, ss.str().c_str(), &strWidth, nullptr);
      Gui::drawTextAligned(font20, 310, 285, currTheme.textColor, ss.str().c_str(), ALIGNED_LEFT);

      if (cursorBlinkCnt++ % 20 > 10 && m_selectedEntry == 0)
        Gui::drawRectangled(312 + strWidth, 285, 3, 35, currTheme.highlightColor);

      if (m_searchValueFormat == FORMAT_DEC)
        Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 100, Gui::g_framebuffer_height - 100, currTheme.textColor, "\uE0E2 Hexadecimal view     \uE0E1 Back     \uE0E0 OK", ALIGNED_RIGHT);
      else if (m_searchValueFormat == FORMAT_HEX)
        Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 100, Gui::g_framebuffer_height - 100, currTheme.textColor, "\uE0E2 Decimal view     \uE0E1 Back     \uE0E0 OK", ALIGNED_RIGHT);
      
      if (m_selectedEntry == 1)
        Gui::drawRectangled(Gui::g_framebuffer_width / 2 - 155, 345, 310, 90, currTheme.highlightColor);

      if (m_searchType != SEARCH_TYPE_NONE && m_searchMode != SEARCH_MODE_NONE && m_searchRegion != SEARCH_REGION_NONE) {
        Gui::drawRectangled(Gui::g_framebuffer_width / 2 - 150, 350, 300, 80, currTheme.selectedColor);
        Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 2, 375, currTheme.backgroundColor, "Search Now!", ALIGNED_CENTER);
      }
      else {
        Gui::drawRectangled(Gui::g_framebuffer_width / 2 - 150, 350, 300, 80, currTheme.selectedButtonColor);
        Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 2, 375, currTheme.separatorColor, "Search Now!", ALIGNED_CENTER);
      }
      

      break;
    case SEARCH_NONE: break;
  }
}

void GuiRAMEditor::onInput(u32 kdown) {
  if (kdown & KEY_B) {

    if (m_searchMenuLocation == SEARCH_NONE) {
      Gui::g_nextGui = GUI_MAIN;
      return;
    }
    else if (m_searchMenuLocation == SEARCH_TYPE) {
      if (m_searchType == SEARCH_TYPE_NONE && m_foundAddresses->size() == 0)
        m_searchMenuLocation = SEARCH_NONE;
      else 
        m_searchType = SEARCH_TYPE_NONE;
    }
    else if (m_searchMenuLocation == SEARCH_MODE) {
      if (m_searchMode == SEARCH_MODE_NONE)
        m_searchMenuLocation = SEARCH_NONE;
      else 
        m_searchMode = SEARCH_MODE_NONE;
    }
    else if (m_searchMenuLocation == SEARCH_REGION) {
      if (m_searchRegion == SEARCH_REGION_NONE && m_foundAddresses->size() == 0)
        m_searchMenuLocation = SEARCH_NONE;
      else 
        m_searchRegion = SEARCH_REGION_NONE;
    }
    else if (m_searchMenuLocation == SEARCH_VALUE)
      m_searchMenuLocation = SEARCH_NONE;
  }

  if (m_debugger->getRunningApplicationPID() == 0)
    return;

  if (m_searchMenuLocation == SEARCH_NONE) {
    if (kdown & KEY_UP) {
        if (m_selectedEntry > 0)
          m_selectedEntry--;

        if (m_menuLocation == CHEATS)
          if (m_selectedEntry == cheatListOffset && cheatListOffset > 0)
            cheatListOffset--;
      }
      
      if (kdown & KEY_DOWN) {
        if (m_menuLocation == CANDIDATES) {
          if (m_selectedEntry < 7 && m_selectedEntry < (m_foundAddresses->size() - 1))
            m_selectedEntry++;
        } else {
          if (m_selectedEntry < (m_cheatCnt - 1))
            m_selectedEntry++;

          if (m_selectedEntry == (cheatListOffset + 7) && cheatListOffset < (m_cheatCnt - 8))
            cheatListOffset++;
        }
      }

      if (m_foundAddresses->size() > 0) {
        if (kdown & KEY_LEFT)
          if (m_cheatCnt > 0) {
            m_menuLocation = CHEATS;
            m_selectedEntry = 0;
            cheatListOffset = 0;
          }

        if (kdown & KEY_RIGHT) {
          m_menuLocation = CANDIDATES;
          m_selectedEntry = 0;
          cheatListOffset = 0;
        }
      }

      if (m_menuLocation == CANDIDATES) { /* Candidates menu */
        if (m_foundAddresses->size() != 0) { 
          if (kdown & KEY_X && m_sysmodulePresent) {
            if (!_isAddressFrozen(m_foundAddresses->getAddress(m_selectedEntry).addr)) {
              u64 outValue;

              if (R_SUCCEEDED(dmntchtEnableFrozenAddress(m_foundAddresses->getAddress(m_selectedEntry).addr, dataTypeSizes[m_searchType], &outValue))) {
                (new Snackbar("Froze variable!"))->show();
                m_frozenAddresses.insert({ m_foundAddresses->getAddress(m_selectedEntry).addr, outValue });
              }
              else
                (new Snackbar("Failed to freeze variable!"))->show();
            }
            else {
              if (R_SUCCEEDED(dmntchtDisableFrozenAddress(m_foundAddresses->getAddress(m_selectedEntry).addr))) {
                (new Snackbar("Unfroze variable!"))->show();
                m_frozenAddresses.erase(m_foundAddresses->getAddress(m_selectedEntry).addr);
              }
              else
                (new Snackbar("Failed to unfreeze variable!"))->show();
            }
          }

          if (kdown & KEY_A) {
            if (m_selectedEntry < 7) {
              char input[16];
              char initialString[21];

              strcpy(initialString, _getAddressDisplayString(m_foundAddresses->getAddress(m_selectedEntry).addr, m_debugger, m_searchType).c_str());

              if (Gui::requestKeyboardInput("Enter value", "Enter a value that should get written at this .", initialString, SwkbdType::SwkbdType_NumPad, input, 15)) {
                m_debugger->pokeMemory(dataTypeSizes[m_searchType], m_foundAddresses->getAddress(m_selectedEntry).addr, atol(input));
              }
            }
            else if (m_foundAddresses->size() < 25) {
              std::vector<std::string> options;
              options.clear();

              std::stringstream ss;
              for (u32 i = 7; i < m_foundAddresses->size(); i++) {
                printf("Start\n");
                ss.str("");
                ss << "0x" << std::uppercase << std::hex << std::setfill('0') << std::setw(10) << m_foundAddresses->getAddress(i).addr;

                ss << " (" << _getAddressDisplayString(m_foundAddresses->getAddress(i).addr, m_debugger, m_searchType);

                options.push_back(ss.str());
                printf("%s\n", ss.str().c_str());
              }

              (new ListSelector("Edit value at ", "\uE0E0 Edit value     \uE0E1 Back", options))->setInputAction([&](u32 k, u16 selectedItem) {
                if (k & KEY_A) {
                  char input[16];
                  char initialString[21];

                  strcpy(initialString, _getAddressDisplayString(m_foundAddresses->getAddress(selectedItem + 7).addr, m_debugger, m_searchType).c_str());

                  if (Gui::requestKeyboardInput("Enter value", "Enter a value for which the game's memory should be searched.", initialString, SwkbdType::SwkbdType_NumPad, input, 15)) {
                    u64 value = atol(input);
                    if (value > dataTypeMaxValues[m_searchType] || value < dataTypeMinValues[m_searchType]) {
                      (new Snackbar("Entered value isn't inside the range of this data type. Please enter a different value."))->show();
                      return;
                    }

                    m_debugger->pokeMemory(dataTypeSizes[m_searchType], m_foundAddresses->getAddress(m_selectedEntry).addr, value);
                  }
                }
              })->show();
            } else (new Snackbar("Too many es! Try narrowing down the selection a bit before editing."))->show();
          }
        }
    } else { /* Cheats menu */
      if (kdown & KEY_A) {
        if (m_cheatCnt == 0) return;

        dmntchtToggleCheat(m_cheats[m_selectedEntry].cheat_id);
        u64 cheatCnt = 0;

        dmntchtGetCheatCount(&cheatCnt);
        if (cheatCnt > 0) {
          delete[] m_cheats;
          m_cheats = new DmntCheatEntry[cheatCnt];
          dmntchtGetCheats(m_cheats, cheatCnt, 0, &m_cheatCnt);
        }
      }
    }

    if (kdown & KEY_MINUS) {
      if (m_foundAddresses->size() == 0) {
        std::vector<std::string> options;
        
        if (m_frozenAddresses.size() == 0)
          return;

        std::stringstream ss;
        for (auto [addr, value] : m_frozenAddresses) {
          ss << "[ BASE + 0x" << std::uppercase << std::hex << std::setfill('0') << std::setw(10) << (addr - m_addressSpaceBaseAddr) << " ]  ";
          ss << "( " << std::dec << value << " )";
          options.push_back(ss.str());
          ss.str("");
        }

        (new ListSelector("Frozen Addresses", "\uE0E0 Unfreeze     \uE0E1 Back", options))->setInputAction([&](u32 k, u16 selectedItem) {
          if (k & KEY_A) {
            auto itr = m_frozenAddresses.begin();
            std::advance(itr, selectedItem);
            
            dmntchtDisableFrozenAddress(itr->first);
            m_frozenAddresses.erase(itr->first);
          }
        })->show();
      } else {
        m_foundAddresses->clear();
        m_menuLocation = CHEATS;
      }
    }

    if (kdown & KEY_Y) {
      if (m_searchMenuLocation == SEARCH_NONE) {
        m_searchMenuLocation = SEARCH_TYPE;
        m_selectedEntry = m_searchType == SEARCH_TYPE_NONE ? 0 : m_searchType;
        cheatListOffset = 0;
      }
    }
  }
  else {
    if ((m_searchMenuLocation == SEARCH_TYPE && m_searchType == SEARCH_TYPE_NONE) ||
        (m_searchMenuLocation == SEARCH_MODE && m_searchMode == SEARCH_MODE_NONE) ||
        (m_searchMenuLocation == SEARCH_REGION && m_searchRegion == SEARCH_REGION_NONE) ||
        (m_searchMenuLocation == SEARCH_VALUE)) {
      if (kdown & KEY_UP) {
        switch (m_searchMenuLocation) {
          case SEARCH_TYPE: [[fallthrough]]
          case SEARCH_MODE:
            if (m_selectedEntry % 2 == 1)
              m_selectedEntry--;
            break;
          case SEARCH_REGION:
            if (m_selectedEntry > 0)
              m_selectedEntry--;
            break;
          case SEARCH_VALUE:
            m_selectedEntry = 0;
            break;
          case SEARCH_NONE: break;
        }
      }

      if (kdown & KEY_DOWN) {
        switch (m_searchMenuLocation) {
          case SEARCH_TYPE: [[fallthrough]]
          case SEARCH_MODE:
            if ((m_selectedEntry + 1) < 10 && m_selectedEntry % 2 == 0)
              m_selectedEntry++;
            break;
          case SEARCH_REGION:
            if (m_selectedEntry < 3)
              m_selectedEntry++;
            break;
          case SEARCH_VALUE:
            if (m_searchType != SEARCH_TYPE_NONE && m_searchMode != SEARCH_MODE_NONE && m_searchRegion != SEARCH_REGION_NONE)
              m_selectedEntry = 1;
            break;
          case SEARCH_NONE: break;
        }
      }

      if (kdown & KEY_LEFT) {
        switch (m_searchMenuLocation) {
          case SEARCH_TYPE: [[fallthrough]]
          case SEARCH_MODE:
            if (m_selectedEntry >= 2)
              m_selectedEntry -= 2;
            break;
          case SEARCH_REGION:
            break;
          case SEARCH_VALUE:
            break;
          case SEARCH_NONE: break;
        }
      }

      if (kdown & KEY_RIGHT) {
        switch (m_searchMenuLocation) {
          case SEARCH_TYPE: [[fallthrough]]
          case SEARCH_MODE:
            if (m_selectedEntry <= 8)
              m_selectedEntry += 2;
            break;
          case SEARCH_REGION:
            break;
          case SEARCH_VALUE:
            break;
          case SEARCH_NONE: break;
        }
      }

      if (kdown & KEY_A) {
        if (m_searchMenuLocation == SEARCH_TYPE)
          m_searchType = static_cast<searchType_t>(m_selectedEntry);
        else if (m_searchMenuLocation == SEARCH_REGION)
          m_searchRegion = static_cast<searchRegion_t>(m_selectedEntry);
        else if (m_searchMenuLocation == SEARCH_MODE)
          m_searchMode = static_cast<searchMode_t>(m_selectedEntry);
        else if (m_searchMenuLocation == SEARCH_VALUE) {
          if (m_selectedEntry == 0) {
            char str[0x21];
            Gui::requestKeyboardInput("Enter the value you want to search for", "Based on your previously chosen options, EdiZon will expect different input here.", "", m_searchValueFormat == FORMAT_DEC ? SwkbdType_NumPad : SwkbdType_QWERTY, str, 0x20);

            if (std::string(str) == "") return;

            if (m_searchValueFormat == FORMAT_HEX) {
              m_searchValue._u64 = std::stoul(str, nullptr, 0);
            } else {
              switch(m_searchType) {
                case SEARCH_TYPE_UNSIGNED_8BIT:
                case SEARCH_TYPE_UNSIGNED_16BIT:
                case SEARCH_TYPE_UNSIGNED_32BIT:
                case SEARCH_TYPE_UNSIGNED_64BIT:
                  m_searchValue._u64 = static_cast<u64>(std::stoul(str, nullptr, 0));
                  break;
                case SEARCH_TYPE_SIGNED_8BIT:
                case SEARCH_TYPE_SIGNED_16BIT:
                case SEARCH_TYPE_SIGNED_32BIT:
                case SEARCH_TYPE_SIGNED_64BIT:
                  m_searchValue._s64 = static_cast<s64>(std::stol(str, nullptr, 0));
                  break;
                case SEARCH_TYPE_FLOAT_32BIT:
                  m_searchValue._f32 = static_cast<float>(std::stof(str));
                  break;
                case SEARCH_TYPE_FLOAT_64BIT:
                  m_searchValue._f64 = static_cast<double>(std::stod(str));
                  break;
                case SEARCH_TYPE_POINTER:
                  m_searchValue._u64 = static_cast<u64>(std::stol(str));
                  break;
                case SEARCH_TYPE_NONE: break;
              }
            }
          } else if (m_selectedEntry == 1) {
            (new MessageBox("Traversing title memory. \n This may take a while...", MessageBox::NONE))->show();
            requestDraw();

            delete m_foundAddresses;
            m_foundAddresses = new MemoryDump("/switch/EdiZon/memdump.dat", m_heapBaseAddr, m_searchValue, m_searchType, m_searchRegion, m_foundAddresses->size() == 0);

            
            if (m_foundAddresses->size() == 0) {
              std::thread t(&GuiRAMEditor::searchMemoryBegin, this, m_debugger, m_searchValue, (searchValue_t){ 0 }, m_searchType, m_searchMode, m_searchRegion, m_foundAddresses, m_memoryInfo);
              t.join();
            }
            else {
              std::thread t(&GuiRAMEditor::searchMemoryContinue, this, m_debugger, m_searchValue, (searchValue_t){ 0 }, m_searchType, m_searchMode, m_foundAddresses);
              t.join();
            }

            Gui::g_currMessageBox->hide();

            m_searchMenuLocation = SEARCH_NONE;
            m_searchMode = SEARCH_MODE_NONE;
          }
        }
      }
    }

    if (kdown & KEY_X) {
      if (m_searchMenuLocation == SEARCH_VALUE) {
        if (m_searchValueFormat == FORMAT_DEC)
          m_searchValueFormat = FORMAT_HEX;
        else
          m_searchValueFormat = FORMAT_DEC;
      }
    }

    if (kdown & KEY_L) {
      if (m_searchMenuLocation == SEARCH_VALUE) {
        m_searchMenuLocation = SEARCH_REGION;
        m_selectedEntry = m_searchRegion == SEARCH_REGION_NONE ? 0 : static_cast<u32>(m_searchRegion);
      }
      else if (m_searchMenuLocation == SEARCH_REGION) {
        m_searchMenuLocation = SEARCH_MODE;
        m_selectedEntry = m_searchMode == SEARCH_MODE_NONE ? 0 : static_cast<u32>(m_searchMode);
      }
      else if (m_searchMenuLocation == SEARCH_MODE) {
        m_searchMenuLocation = SEARCH_TYPE;
        m_selectedEntry = m_searchType == SEARCH_TYPE_NONE ? 0 : static_cast<u32>(m_searchType);
      }
    }

    if (kdown & KEY_R) {
      if (m_searchMenuLocation == SEARCH_TYPE) {
        m_searchMenuLocation = SEARCH_MODE;
        m_selectedEntry = m_searchMode == SEARCH_MODE_NONE ? 0 : static_cast<u32>(m_searchMode);
      }
      else if (m_searchMenuLocation == SEARCH_MODE) {
        m_searchMenuLocation = SEARCH_REGION;
        m_selectedEntry = m_searchRegion == SEARCH_REGION_NONE ? 0 : static_cast<u32>(m_searchRegion);
      }
      else if (m_searchMenuLocation == SEARCH_REGION) {
        m_searchMenuLocation = SEARCH_VALUE;
        m_selectedEntry = 0;
      }
    }

  }
}

void GuiRAMEditor::onTouch(touchPosition &touch) {

}

void GuiRAMEditor::onGesture(touchPosition startPosition, touchPosition endPosition, bool finish) {

}

bool _isAddressFrozen(uintptr_t address) {
  DmntFrozenAddressEntry *es;
  u64 Cnt = 0;
  bool frozen = false;

  dmntchtGetFrozenAddressCount(&Cnt);

  if (Cnt != 0) {
    es = new DmntFrozenAddressEntry[Cnt];
    dmntchtGetFrozenAddresses(es, Cnt, 0, nullptr);

    for (u64 i = 0; i < Cnt; i++) {
      if (es[i].address == address) {
        frozen = true;
        break;
      }
    }
  }

  return frozen;
}

std::string _getAddressDisplayString(u64 address, Debugger *debugger, searchType_t searchType) {
  std::stringstream ss;

  searchValue_t searchValue;
  searchValue._u64 = debugger->peekMemory(address);

  switch(searchType) {
    case SEARCH_TYPE_UNSIGNED_8BIT:
      ss << std::dec << static_cast<u64>(searchValue._u8);
      break;
    case SEARCH_TYPE_UNSIGNED_16BIT:
      ss << std::dec << static_cast<u64>(searchValue._u16);
      break;
    case SEARCH_TYPE_UNSIGNED_32BIT:
      ss << std::dec << static_cast<u64>(searchValue._u32);
      break;
    case SEARCH_TYPE_UNSIGNED_64BIT:
      ss << std::dec << static_cast<u64>(searchValue._u64);
      break;
    case SEARCH_TYPE_SIGNED_8BIT:
      ss << std::dec << static_cast<s64>(searchValue._s8);
      break;
    case SEARCH_TYPE_SIGNED_16BIT:
      ss << std::dec << static_cast<s64>(searchValue._s16);
      break;
    case SEARCH_TYPE_SIGNED_32BIT:
      ss << std::dec << static_cast<s64>(searchValue._s32);
      break;
    case SEARCH_TYPE_SIGNED_64BIT:
      ss << std::dec << static_cast<s64>(searchValue._s64);
      break;
    case SEARCH_TYPE_FLOAT_32BIT:
      ss << std::dec << searchValue._f32;
      break;
    case SEARCH_TYPE_FLOAT_64BIT:
      ss << std::dec << searchValue._f64;
      break;
    case SEARCH_TYPE_POINTER:
      ss << std::dec << searchValue._u64;
      break;
    case SEARCH_TYPE_NONE: break;
  }

  return ss.str();
}

std::string _getValueDisplayString(searchValue_t searchValue, searchType_t searchType) {
  std::stringstream ss;

  switch(searchType) {
    case SEARCH_TYPE_UNSIGNED_8BIT:
      ss << std::dec << static_cast<u64>(searchValue._u8);
      break;
    case SEARCH_TYPE_UNSIGNED_16BIT:
      ss << std::dec << static_cast<u64>(searchValue._u16);
      break;
    case SEARCH_TYPE_UNSIGNED_32BIT:
      ss << std::dec << static_cast<u64>(searchValue._u32);
      break;
    case SEARCH_TYPE_UNSIGNED_64BIT:
      ss << std::dec << static_cast<u64>(searchValue._u64);
      break;
    case SEARCH_TYPE_SIGNED_8BIT:
      ss << std::dec << static_cast<s64>(searchValue._s8);
      break;
    case SEARCH_TYPE_SIGNED_16BIT:
      ss << std::dec << static_cast<s64>(searchValue._s16);
      break;
    case SEARCH_TYPE_SIGNED_32BIT:
      ss << std::dec << static_cast<s64>(searchValue._s32);
      break;
    case SEARCH_TYPE_SIGNED_64BIT:
      ss << std::dec << static_cast<s64>(searchValue._s64);
      break;
    case SEARCH_TYPE_FLOAT_32BIT:
      ss << std::dec << searchValue._f32;
      break;
    case SEARCH_TYPE_FLOAT_64BIT:
      ss << std::dec << searchValue._f64;
      break;
    case SEARCH_TYPE_POINTER:
      ss << std::dec << searchValue._u64;
      break;
    case SEARCH_TYPE_NONE: break;
  }

  return ss.str();
}

void GuiRAMEditor::searchMemoryBegin(Debugger *debugger, searchValue_t searchValue1, searchValue_t searchValue2, searchType_t searchType, searchMode_t searchMode, searchRegion_t searchRegion, MemoryDump *foundAddrs, std::vector<MemoryInfo> memInfos) {
  for (MemoryInfo meminfo : memInfos) {
    if (searchRegion == SEARCH_REGION_HEAP && meminfo.type != MemType_Heap)
     continue;
    else if (searchRegion == SEARCH_REGION_MAIN &&
     (meminfo.type != MemType_CodeWritable && meminfo.type != MemType_CodeMutable))
      continue;
    else if (searchRegion == SEARCH_REGION_HEAP_AND_MAIN &&
     (meminfo.type != MemType_Heap && meminfo.type != MemType_CodeWritable && meminfo.type != MemType_CodeMutable))
      continue;
    else if (searchRegion == SEARCH_REGION_RAM && (meminfo.perm & Perm_Rw) != Perm_Rw)
      continue;

    u64 offset = 0;
    u64 bufferSize = 0x10000;
    u8 *buffer = new u8[bufferSize];
    while (offset < meminfo.size) {
      
      if (meminfo.size - offset < bufferSize)
        bufferSize = meminfo.size - offset;

      debugger->readMemory(buffer, bufferSize, meminfo.addr + offset);

      searchValue_t realValue = { 0 };
      for (u64 i = 0; i < bufferSize; i += dataTypeSizes[searchType]) {
        memset(&realValue, 0, 8);
        memcpy(&realValue, buffer + i, dataTypeSizes[searchType]);

        switch(searchMode) {
          case SEARCH_MODE_EQ:
            if (realValue._s64 == searchValue1._s64)
              foundAddrs->pushAddress({ .addr = meminfo.addr + offset + i, .type = (MemoryType) meminfo.type });
            break;
          case SEARCH_MODE_NEQ:
            if (realValue._s64 != searchValue1._s64)
              foundAddrs->pushAddress({ .addr = meminfo.addr + offset + i, .type = (MemoryType) meminfo.type });
            break;
          case SEARCH_MODE_GT:
            if (realValue._s64 > searchValue1._s64)
              foundAddrs->pushAddress({ .addr = meminfo.addr + offset + i, .type = (MemoryType) meminfo.type });
            break;
          case SEARCH_MODE_GTE:
            if (realValue._s64 >= searchValue1._s64)
              foundAddrs->pushAddress({ .addr = meminfo.addr + offset + i, .type = (MemoryType) meminfo.type });
            break;
          case SEARCH_MODE_LT:
            if (realValue._s64 < searchValue1._s64)
              foundAddrs->pushAddress({ .addr = meminfo.addr + offset + i, .type = (MemoryType) meminfo.type });
            break;
          case SEARCH_MODE_LTE:
            if (realValue._s64 <= searchValue1._s64)
              foundAddrs->pushAddress({ .addr = meminfo.addr + offset + i, .type = (MemoryType) meminfo.type });
            break;
          case SEARCH_MODE_RANGE:
            if (realValue._s64 >= searchValue1._s64 && realValue._s64 <= searchValue2._s64)
              foundAddrs->pushAddress({ .addr = meminfo.addr + offset + i, .type = (MemoryType) meminfo.type });
            break;

          case SEARCH_MODE_SAME: [[fallthrough]]
          case SEARCH_MODE_DIFF: [[fallthrough]]
          case SEARCH_MODE_INC:  [[fallthrough]]
          case SEARCH_MODE_DEC:
            foundAddrs->pushAddress({ .addr = meminfo.addr + offset + i, .type = (MemoryType) meminfo.type });
            break;
          case SEARCH_MODE_NONE: break;
        }
      }

      offset += bufferSize;
    }

    delete[] buffer;
  }

  foundAddrs->flush();
}

void GuiRAMEditor::searchMemoryContinue(Debugger *debugger, searchValue_t searchValue1, searchValue_t searchValue2, searchType_t searchType, searchMode_t searchMode, MemoryDump *foundAddrs) {
  std::vector<ramAddr_t> newAddresses;
  for (s64 i = 0; i < foundAddrs->size(); i++) {
    searchValue_t value = { 0 };

    debugger->readMemory(&value, dataTypeSizes[searchType], foundAddrs->getAddress(i).addr);

    switch(searchMode) {
      case SEARCH_MODE_EQ:
        if (value._s64 == searchValue1._s64)
          newAddresses.push_back(foundAddrs->getAddress(i));
        break;
      case SEARCH_MODE_NEQ:
        if (value._s64 != searchValue1._s64)
          newAddresses.push_back(foundAddrs->getAddress(i));
        break;
      case SEARCH_MODE_GT:
        if (value._s64 > searchValue1._s64)
          newAddresses.push_back(foundAddrs->getAddress(i));
        break;
      case SEARCH_MODE_GTE:
        if (value._s64 >= searchValue1._s64)
          newAddresses.push_back(foundAddrs->getAddress(i));
        break;
      case SEARCH_MODE_LT:
        if (value._s64 < searchValue1._s64)
          newAddresses.push_back(foundAddrs->getAddress(i));
        break;
      case SEARCH_MODE_LTE:
        if (value._s64 <= searchValue1._s64)
          newAddresses.push_back(foundAddrs->getAddress(i));
        break;
      case SEARCH_MODE_RANGE:
        if (value._s64 >= searchValue1._s64 && value._s64 <= searchValue2._s64)
          newAddresses.push_back(foundAddrs->getAddress(i));
        break;
      case SEARCH_MODE_SAME:
        if (value._s64 == searchValue1._s64)
          newAddresses.push_back(foundAddrs->getAddress(i));
        break;
      case SEARCH_MODE_DIFF:
        if (value._s64 != searchValue1._s64)
          newAddresses.push_back(foundAddrs->getAddress(i));
        break;
      case SEARCH_MODE_INC:
        if (value._s64 > searchValue1._s64)
          newAddresses.push_back(foundAddrs->getAddress(i));
        break;
      case SEARCH_MODE_DEC:
        if (value._s64 < searchValue1._s64)
          newAddresses.push_back(foundAddrs->getAddress(i));
        break;
      case SEARCH_MODE_NONE: break;
    }
  }

  if (newAddresses.size() > 0) {
    foundAddrs->setSearchValue(searchValue1);
    foundAddrs->clearAddresses();
    for (u64 i = 0; i < newAddresses.size(); i++) {
      foundAddrs->pushAddress(newAddresses[i]);
    }    
  }
  
  foundAddrs->flush();
}