#include "guis/gui_ram_editor.hpp"

#include <sstream>

static std::vector<std::string> dataTypes = { "s8", "u8", "s16", "u16", "s32", "u32", "s64", "u64", "f32", "f64", "ptr", "str" };
static std::vector<u8> dataTypeSizes      = {    1,   1,     2,     2,     4,     4,     8,     8,     4,     8,     8,     0  };
static std::string titleNameStr, tidStr, pidStr;


GuiRAMEditor::GuiRAMEditor() : Gui() {
  m_searchMode = SEARCH_BEGIN;
  m_searchType = SIGNED_8BIT;

  if (m_debugger.getRunningApplicationPID() == 0) {
    remove("/EdiZon/cheats/addresses.dat");
    return;
  }

  m_debugger.attachToProcess();

  MemoryInfo meminfo = { 0 };
  u64 lastAddr = 0;

  do {
    lastAddr = meminfo.addr;
    meminfo = m_debugger.queryMemory(meminfo.addr + meminfo.size);

    m_memoryInfo.push_back(meminfo);
  } while (lastAddr < meminfo.addr + meminfo.size);

  m_debugger.continueProcess();

  for (MemoryInfo meminfo : m_memoryInfo) {
    for (u64 addrOffset = meminfo.addr; addrOffset < meminfo.addr + meminfo.size; addrOffset += 0x20000000) {
      switch(meminfo.type) {
        case MemType_CodeReadOnly:
        case MemType_CodeWritable:
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

  u64 oldPid = 0;
  size_t addressCnt = 0;
  std::ifstream file("/EdiZon/cheats/addresses.dat", std::ios::in | std::ios::binary);

  if (file.is_open()) {
    m_searchMode = SEARCH_CONTINUE;
    file.read((char*)&addressCnt, 8);
    file.read((char*)&m_searchType, 1);
    file.read((char*)&oldPid, 8);

    u64 *buffer = new u64[addressCnt];
    m_foundAddresses.reserve(addressCnt);

    file.read((char*)buffer, addressCnt * sizeof(u64));

    std::copy(buffer, buffer + addressCnt, std::back_inserter(m_foundAddresses));

    delete[] buffer;

    file.close();
  }

  if (m_debugger.getRunningApplicationPID() != oldPid && oldPid != 0) {
    m_searchMode = SEARCH_BEGIN;
    remove("/EdiZon/cheats/addresses.dat");
    m_foundAddresses.clear();
    m_searchType = 0;
    Gui::g_nextGui = GUI_MAIN;
  }

  std::stringstream ss;

  if (Title::g_titles[m_debugger.getRunningApplicationTID()]->getTitleName().length() < 24)
    ss << Title::g_titles[m_debugger.getRunningApplicationTID()]->getTitleName();
  else
    ss << Title::g_titles[m_debugger.getRunningApplicationTID()]->getTitleName().substr(0, 21) << "...";
  titleNameStr = ss.str();
  ss.str("");

  ss << "TID: " << std::uppercase << std::hex << std::setfill('0') << std::setw(sizeof(u64) * 2) << m_debugger.getRunningApplicationTID();
  tidStr = ss.str();
  ss.str("");

  ss << "PID: " << std::dec << m_debugger.getRunningApplicationPID();
  pidStr = ss.str();
}

GuiRAMEditor::~GuiRAMEditor() {
    m_debugger.detachFromProcess();
}



void GuiRAMEditor::update() {
  Gui::update();
}

void GuiRAMEditor::draw() {

  Gui::beginDraw();

  Gui::drawRectangle(0, 0, Gui::g_framebuffer_width, Gui::g_framebuffer_height, currTheme.backgroundColor);

  Gui::drawRectangle((u32)((Gui::g_framebuffer_width - 1220) / 2), Gui::g_framebuffer_height - 73, 1220, 1, currTheme.textColor);

  if (m_debugger.getRunningApplicationPID() == 0) {
    Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2 - 50, currTheme.textColor, "To use the RAM editor, a title has to be running in the background. \n Please launch a game and try again.", ALIGNED_CENTER);
    Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 50, Gui::g_framebuffer_height - 50, currTheme.textColor, "\uE0EF Exit     \uE0E1 Back", ALIGNED_RIGHT);
    Gui::endDraw();
    return;
  }

  if (m_searchMode == SEARCH_BEGIN)
    Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 50, Gui::g_framebuffer_height - 50, currTheme.textColor, "\uE0EF Exit     \uE0E2 Search RAM     \uE0E1 Back", ALIGNED_RIGHT);
  else if (m_searchMode == SEARCH_CONTINUE) {
    if (m_foundAddresses.size() > 0)
      Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 50, Gui::g_framebuffer_height - 50, currTheme.textColor, "\uE0EF Exit     \uE0F0 Reset search     \uE0E2 Search again     \uE0E0 Edit value     \uE0E1 Back", ALIGNED_RIGHT);
    else 
      Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 50, Gui::g_framebuffer_height - 50, currTheme.textColor, "\uE0EF Exit     \uE0F0 Reset search     \uE0E1 Back", ALIGNED_RIGHT);
  }

  Gui::drawRectangle(256, 50, Gui::g_framebuffer_width - 256, 206, currTheme.separatorColor);
  Gui::drawImage(0, 0, 256, 256, Title::g_titles[m_debugger.getRunningApplicationTID()]->getTitleIcon(), IMAGE_MODE_RGB24);


  Gui::drawRectangle(256, 50, 184, 70, currTheme.selectedColor);
  for (u8 i = 0; i < 12; i++) {
    Gui::drawRectangle(440 + i * 70, 50, 70, 70, currTheme.textColor);
    Gui::drawRectangle(440 + i * 70 + 2, 50 + 2, 70 - 4, 70 - 4, m_searchType == i ? m_searchMode == SEARCH_BEGIN ? currTheme.highlightColor : currTheme.selectedColor : currTheme.separatorColor);
    Gui::drawTextAligned(font20, 475 + i * 70 + 2, 67, m_searchType == i ? COLOR_BLACK : currTheme.textColor, dataTypes[i].c_str(), ALIGNED_CENTER);
  }
  Gui::drawTextAligned(font20, 280, 67, COLOR_BLACK, "Data type", ALIGNED_LEFT);

  Gui::drawRectangle(256, 119, 400, 137, currTheme.selectedColor);

  Gui::drawTextAligned(font20, 280, 140, COLOR_BLACK, titleNameStr.c_str(), ALIGNED_LEFT);
  Gui::drawTextAligned(font20, 280, 170, COLOR_BLACK, tidStr.c_str(), ALIGNED_LEFT);
  Gui::drawTextAligned(font20, 280, 200, COLOR_BLACK, pidStr.c_str(), ALIGNED_LEFT);

  for (u8 line = 0; line < 9; line++) {
    if (line >= m_foundAddresses.size()) break;

    std::stringstream ss;

    if (line < 8 && m_foundAddresses.size() != 9) {
      ss << "0x" << std::uppercase << std::hex << std::setfill('0') << std::setw(10) << m_foundAddresses[line];

      float fcast = 0.0F;
      double dcast = 0.0;
      u64 fvalue = 0;
      u64 dvalue = 0;

      switch(m_searchType) {
        case UNSIGNED_8BIT:
          ss << " (" << std::dec << static_cast<u8>(m_debugger.peekMemory(m_foundAddresses[line])) << ")";
          break;
        case UNSIGNED_16BIT:
          ss << " (" << std::dec << static_cast<u16>(m_debugger.peekMemory(m_foundAddresses[line])) << ")";
          break;
        case UNSIGNED_32BIT:
          ss << " (" << std::dec << static_cast<u32>(m_debugger.peekMemory(m_foundAddresses[line])) << ")";
          break;
        case UNSIGNED_64BIT:
          ss << " (" << std::dec << static_cast<u64>(m_debugger.peekMemory(m_foundAddresses[line])) << ")";
          break;
        case SIGNED_8BIT:
          ss << " (" << std::dec << static_cast<s8>(m_debugger.peekMemory(m_foundAddresses[line])) << ")";
          break;
        case SIGNED_16BIT:
          ss << " (" << std::dec << static_cast<s16>(m_debugger.peekMemory(m_foundAddresses[line])) << ")";
          break;
        case SIGNED_32BIT:
          ss << " (" << std::dec << static_cast<s32>(m_debugger.peekMemory(m_foundAddresses[line])) << ")";
          break;
        case SIGNED_64BIT:
          ss << " (" << std::dec << static_cast<s64>(m_debugger.peekMemory(m_foundAddresses[line])) << ")";
          break;
        case FLOAT_32BIT:
          fvalue = m_debugger.peekMemory(m_foundAddresses[line]);
          memcpy(&fcast, &fvalue, 4);
          ss << " (" << fcast << ")";
          break;
        case FLOAT_64BIT:
          dvalue = m_debugger.peekMemory(m_foundAddresses[line]);
          memcpy(&dcast, &dvalue, 8);
          ss << " (" << dcast << ")"; 
          break;
      }
    }
    else 
      ss << "And " << std::dec << (m_foundAddresses.size() - 8) << " others...";

    Gui::drawRectangle(Gui::g_framebuffer_width - 400, 256 + line * 40, 400, 40, m_selectedAddress == line ? currTheme.highlightColor : line % 2 ? currTheme.backgroundColor : currTheme.separatorColor);
    Gui::drawTextAligned(font14, Gui::g_framebuffer_width - 380, 266 + line * 40, m_selectedAddress == line ? COLOR_BLACK : currTheme.textColor, ss.str().c_str(), ALIGNED_LEFT);
  }

  Gui::drawShadow(0, 0, Gui::g_framebuffer_width, 256);
  Gui::drawShadow(256, 0, Gui::g_framebuffer_width - 256, 50);
  Gui::drawShadow(256, 50, Gui::g_framebuffer_width - 256, 70);

  for (u16 x = 0; x < 1024; x++)
    Gui::drawRectangle(256 + x, 0, 1, 50, m_memory[x]);


  /*std::string strFoundAddresses = "Found " + std::to_string(m_foundAddresses.size()) + " addresses!";
  Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 50, 100, currTheme.textColor, strFoundAddresses.c_str(), ALIGNED_RIGHT);

  u16 offset = 0;
  for (u64 addr : m_foundAddresses) {
    std::stringstream ss;

    if (offset < 425) {
      ss << "0x" << std::uppercase << std::hex << std::setfill('0') << std::setw(10) << addr << "";
      Gui::drawTextAligned(font14, Gui::g_framebuffer_width - 50, 150 + offset, currTheme.textColor, ss.str().c_str(), ALIGNED_RIGHT);
      offset += 25;
    }
    else {
      ss << "...";
      Gui::drawTextAligned(font14, Gui::g_framebuffer_width - 50, 150 + offset, currTheme.textColor, ss.str().c_str(), ALIGNED_RIGHT);
      break;
    }



  }*/

  //Row -
  //Column |

  /*for (u8 column = 0; column < 0x10; column++) {
    ss.str("");
    ss << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << static_cast<u16>(column);
    Gui::drawTextAligned(font14, 450 + column * 50, 50, currTheme.textColor, ss.str().c_str(), ALIGNED_LEFT);
   
    for (u8 row = 0; row < 14; row++) {
      ss.str("");
      ss << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << static_cast<u16>(m_ramBuffer[row * 0x10 + column]);
      Gui::drawTextAligned(font14, 450 + column * 50, 100 + row * 40, currTheme.separatorColor, ss.str().c_str(), ALIGNED_LEFT);

      if (column == 0) {
        ss.str("");
        ss << "0x" << std::uppercase << std::setfill('0') << std::setw(9) << std::hex << static_cast<u64>(m_ramAddress + row * 0x10);
        Gui::drawTextAligned(font14, 300, 100 + row * 40, currTheme.textColor, ss.str().c_str(), ALIGNED_LEFT);
      }
    }
  }*/


  Gui::endDraw();
}

void GuiRAMEditor::onInput(u32 kdown) {
  if (kdown & KEY_B) 
    Gui::g_nextGui = GUI_MAIN;

  if (m_debugger.getRunningApplicationPID() == 0)
    return;

  if (m_searchMode == SEARCH_BEGIN) {
    if (kdown & KEY_L)
      if (m_searchType > 0)
        m_searchType--;

    if (kdown & KEY_R)
      if (m_searchType < 11)
        m_searchType++;
    } else {
      if (m_foundAddresses.size() > 0) {
        if (kdown & KEY_UP)
          if (m_selectedAddress > 0)
            m_selectedAddress--;
        
        if (kdown & KEY_DOWN)
          if (m_selectedAddress < 8 && m_selectedAddress < (m_foundAddresses.size() - 1))
            m_selectedAddress++;

        if (kdown & KEY_A) {
          if (m_selectedAddress < 8) {
            char input[16];
            if (Gui::requestKeyboardInput("Enter value", "Enter a value that should get written at this address.", "", SwkbdType::SwkbdType_NumPad, input, 15))
              m_debugger.pokeMemory(dataTypeSizes[m_searchType], m_foundAddresses[m_selectedAddress], atol(input));
          } else if (m_foundAddresses.size() < 25) {
            std::vector<std::string> options;
            options.clear();

            std::stringstream ss;
            for (u32 i = 8; i < m_foundAddresses.size(); i++) {
              ss.str("");
              ss << "0x" << std::uppercase << std::hex << std::setfill('0') << std::setw(10) << m_foundAddresses[i];

              float fcast = 0.0F;
              double dcast = 0.0;
              u64 fvalue = 0;
              u64 dvalue = 0;
              
              switch(m_searchType) {
                case UNSIGNED_8BIT:
                  ss << " (" << std::dec << static_cast<u8>(m_debugger.peekMemory(m_foundAddresses[i])) << ")";
                  break;
                case UNSIGNED_16BIT:
                  ss << " (" << std::dec << static_cast<u16>(m_debugger.peekMemory(m_foundAddresses[i])) << ")";
                  break;
                case UNSIGNED_32BIT:
                  ss << " (" << std::dec << static_cast<u32>(m_debugger.peekMemory(m_foundAddresses[i])) << ")";
                  break;
                case UNSIGNED_64BIT:
                  ss << " (" << std::dec << static_cast<u64>(m_debugger.peekMemory(m_foundAddresses[i])) << ")";
                  break;
                case SIGNED_8BIT:
                  ss << " (" << std::dec << static_cast<s8>(m_debugger.peekMemory(m_foundAddresses[i])) << ")";
                  break;
                case SIGNED_16BIT:
                  ss << " (" << std::dec << static_cast<s16>(m_debugger.peekMemory(m_foundAddresses[i])) << ")";
                  break;
                case SIGNED_32BIT:
                  ss << " (" << std::dec << static_cast<s32>(m_debugger.peekMemory(m_foundAddresses[i])) << ")";
                  break;
                case SIGNED_64BIT:
                  ss << " (" << std::dec << static_cast<s64>(m_debugger.peekMemory(m_foundAddresses[i])) << ")";
                  break;
                case FLOAT_32BIT:
                  fvalue = m_debugger.peekMemory(m_foundAddresses[i]);
                  memcpy(&fcast, &fvalue, 4);
                  ss << " (" << fcast << ")";
                  break;
                case FLOAT_64BIT:
                  dvalue = m_debugger.peekMemory(m_foundAddresses[i]);
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
                  m_debugger.pokeMemory(dataTypeSizes[m_searchType], m_foundAddresses[m_selectedAddress], atol(input));
                }
              }
            })->show();
          } else (new Snackbar("Too many addresses! Try narrowing down the selection a bit before editing."))->show();
        }
      }
    }

  if (kdown & KEY_MINUS) {
    m_searchMode = SEARCH_BEGIN;
    m_foundAddresses.clear();
    remove("/EdiZon/cheats/addresses.dat");
  }

  if (kdown & KEY_X) {
    char input[16];
    if (Gui::requestKeyboardInput("Enter value", "Enter a value for which the game's memory should be searched.", "", SwkbdType::SwkbdType_NumPad, input, 15)) {
      u64 searchValue = atol(input);
      m_debugger.breakProcess();

      (new MessageBox("Searching RAM \n \n This may take a while...", MessageBox::NONE))->show();
      requestDraw();

      if (m_searchMode == SEARCH_CONTINUE) {

        std::vector<u64> newAddresses;
        for (u64 addr : m_foundAddresses) {
          u64 value = 0, realValue = 0;

          m_debugger.readMemory(&value, 8, addr);
          memcpy(&realValue, &value, dataTypeSizes[m_searchType]);

          if (realValue == searchValue) {
            newAddresses.push_back(addr);
          }
        }

        m_foundAddresses.clear();
        std::copy(newAddresses.begin(), newAddresses.end(), std::back_inserter(m_foundAddresses));

        if (m_foundAddresses.size() == 0) {
          m_searchMode = SEARCH_BEGIN;
          remove("/EdiZon/cheats/addresses.dat");
          
          Gui::g_currMessageBox->hide();
          m_debugger.continueProcess();
          (new Snackbar("None of your previously found addresses got changed to the entered value."))->show();
          return;
        }

      } else {
        m_searchMode = SEARCH_CONTINUE;

        for (MemoryInfo meminfo : m_memoryInfo) {
          if (m_searchType != POINTER && meminfo.type != MemType_Heap) continue;
          u64 offset = 0;
          u64 bufferSize = 0x10000;
          u8 *buffer = new u8[bufferSize];
          while (offset < meminfo.size) {
            
            if (meminfo.size - offset < bufferSize)
              bufferSize = meminfo.size - offset;

            m_debugger.readMemory(buffer, bufferSize, meminfo.addr + offset);

            u64 realValue = 0;
            for (u64 i = 0; i < bufferSize; i++) {
              memcpy(&realValue, buffer + i, dataTypeSizes[m_searchType]);

              if (realValue == searchValue) {
                m_foundAddresses.push_back(meminfo.addr + offset + i);
              }
            }

            offset += bufferSize;
          }

          delete[] buffer;
        }
      }

      printf("Done\n");

      std::ofstream file("/EdiZon/cheats/addresses.dat", std::ios::out | std::ios::binary);
      if (file.is_open()) {
        u64 addressCount = m_foundAddresses.size();
        u64 pid = m_debugger.getRunningApplicationPID();
        file.write((char*)&addressCount, 8);
        file.write((char*)&m_searchType, 1);
        file.write((char*)&pid, 8);
        file.write((char*)&m_foundAddresses[0], m_foundAddresses.size() * sizeof(u64));
        file.close();
      } else printf("Didn't save!\n");

      Gui::g_currMessageBox->hide();

      m_debugger.continueProcess();

    }
  }
}

void GuiRAMEditor::onTouch(touchPosition &touch) {

}

void GuiRAMEditor::onGesture(touchPosition startPosition, touchPosition endPosition, bool finish) {

}