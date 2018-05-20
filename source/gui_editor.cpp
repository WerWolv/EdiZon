#include "gui_editor.hpp"
#include "gui_main.hpp"

#include <string>
#include <sstream>

GuiEditor::GuiEditor(Title *title) : Gui() {
  this->m_currentTitle = title;
}

GuiEditor::~GuiEditor() {

}

void GuiEditor::draw() {
  std::stringstream ss;
  ss << "Title ID: 0x" << std::hex << this->m_currentTitle->getTitleID();

  Gui::drawRectangle(0, 0, Gui::m_framebuffer_width, Gui::m_framebuffer_height, currTheme.backgroundColor);
  Gui::drawRectangle(20, 20, Gui::m_framebuffer_width - 40, 256 + 70, currTheme.separatorColor);
  Gui::drawImage(50, 50, 256, 256, this->m_currentTitle->getTitleIcon(), IMAGE_MODE_RGB24);
  Gui::drawText(font24, 350, 60, currTheme.textColor, this->m_currentTitle->getTitleName().c_str());
  Gui::drawText(font20, 370, 100, currTheme.textColor, this->m_currentTitle->getTitleAuthor().c_str());
  Gui::drawText(font20, 370, 160, currTheme.textColor, ss.str().c_str());
  Gui::drawShadow(50, 50, 256, 256);
  Gui::drawShadow(20, 20, Gui::m_framebuffer_width - 40, 256 + 70);

  gfxFlushBuffers();
  gfxSwapBuffers();
  gfxWaitForVsync();
}

void GuiEditor::onInput(u32 kdown) {
  //TODO: MEMORY LEAK!!!
  if(kdown & KEY_B)
    Gui::g_nextGui = new GuiMain();
}
