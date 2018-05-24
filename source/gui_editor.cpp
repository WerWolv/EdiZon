#include "gui_editor.hpp"
#include "gui_main.hpp"

#include <string>
#include <sstream>
#include <map>

u16 width = 0;
u8 errorCode = 0;

GuiEditor::GuiEditor() : Gui() {

}

GuiEditor::~GuiEditor() {

}

void GuiEditor::draw() {
  std::stringstream ss;
  ss << "Title ID: 0x" << std::hex << Title::g_currTitle->getTitleID();

  Gui::drawRectangle(0, 0, Gui::m_framebuffer_width, Gui::m_framebuffer_height, currTheme.backgroundColor);
  Gui::drawRectangle(20, 50, Gui::m_framebuffer_width - 80, 256, currTheme.separatorColor);
  Gui::drawImage(20, 50, 256, 256, Title::g_currTitle->getTitleIcon(), IMAGE_MODE_RGB24);
  Gui::drawText(font24, 350, 60, currTheme.textColor, Title::g_currTitle->getTitleName().c_str());
  Gui::drawText(font20, 370, 100, currTheme.textColor, Title::g_currTitle->getTitleAuthor().c_str());
  Gui::drawText(font20, 370, 160, currTheme.textColor, ss.str().c_str());
  Gui::drawShadow(20, 50, Gui::m_framebuffer_width - 80, 256);
    //Gui::drawText(font20, 500, 500 + (i += 50), currTheme.textColor, account.second.profileBase.username);
  gfxFlushBuffers();
  gfxSwapBuffers();
  gfxWaitForVsync();
}

void GuiEditor::onInput(u32 kdown) {
  if(kdown & KEY_B)
    Gui::g_nextGui = GUI_MAIN;
}

void GuiEditor::onTouch(touchPosition &touch) {

}
