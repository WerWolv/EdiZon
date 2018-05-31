#include "gui_editor.hpp"
#include "gui_main.hpp"

#include <string>
#include <sstream>
#include <map>

#include "save.hpp"

u16 width = 0;
u8 errorCode = 0;

u8* titleIcon;

GuiEditor::GuiEditor() : Gui() {
  titleIcon = (u8*) malloc(128*128*3);

  Gui::resizeImage(Title::g_currTitle->getTitleIcon(), titleIcon, 256, 256, 128, 128);

  m_widgets.push_back({ std::string("Coins"), new WidgetSwitch() });
  m_widgets.push_back({ std::string("Health"), new WidgetSwitch() });
  m_widgets.push_back({ std::string("Power Moons"), new WidgetSwitch() });
}

GuiEditor::~GuiEditor() {
  for(auto widget : m_widgets)
    delete widget.widget;
}

void GuiEditor::draw() {
  std::stringstream ss;
  ss << "0x" << errorCode;

  Gui::drawRectangle(0, 0, Gui::framebuffer_width, Gui::framebuffer_height, currTheme.backgroundColor);
  Gui::drawImage(0, 0, 128, 128, titleIcon, IMAGE_MODE_RGB24);
  Gui::drawImage(Gui::framebuffer_width - 128, 0, 128, 128, Account::g_currAccount->getProfileImage(), IMAGE_MODE_RGB24);
  Gui::drawShadow(0, 0, Gui::framebuffer_width, 128);

  u32 titleWidth, titleHeight;
  Gui::getTextDimensions(font24, Title::g_currTitle->getTitleName().c_str(), &titleWidth, &titleHeight);
  Gui::drawText(font24, (Gui::framebuffer_width / 2) - (titleWidth / 2), 10, currTheme.textColor, Title::g_currTitle->getTitleName().c_str());
  Gui::getTextDimensions(font20, Title::g_currTitle->getTitleAuthor().c_str(), &titleWidth, &titleHeight);
  Gui::drawText(font20, (Gui::framebuffer_width / 2) - (titleWidth / 2), 45, currTheme.textColor, Title::g_currTitle->getTitleAuthor().c_str());
  Gui::getTextDimensions(font20, ss.str().c_str(), &titleWidth, &titleHeight);
  Gui::drawText(font20, (Gui::framebuffer_width / 2) - (titleWidth / 2), 80, currTheme.textColor, std::to_string(errorCode).c_str());

  Widget::drawWidgets(this, m_widgets, 200, 0, 0);

  gfxFlushBuffers();
  gfxSwapBuffers();
  gfxWaitForVsync();
}

void GuiEditor::onInput(u32 kdown) {
  if(kdown & KEY_B)
    Gui::g_nextGui = GUI_MAIN;

  if(kdown & KEY_X) {
    backupSave(Title::g_currTitle->getTitleID(), Account::g_currAccount->getUserID());
  }
  if(kdown & KEY_Y)
    restoreSave(Title::g_currTitle->getTitleID(), Account::g_currAccount->getUserID(), "20180531145819");
}

void GuiEditor::onTouch(touchPosition &touch) {

}
