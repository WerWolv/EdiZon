#include "gui_tx_warning.hpp"

GuiTXWarning::GuiTXWarning() : Gui() {

}

GuiTXWarning::~GuiTXWarning() {

}

void GuiTXWarning::update() {
  Gui::update();
}

void GuiTXWarning::draw() {
  Gui::beginDraw();

  Gui::drawRectangle(0, 0, Gui::g_framebuffer_width, Gui::g_framebuffer_height, Gui::makeColor(0xC5, 0x39, 0x29, 0xFF));
  Gui::drawTextAligned(fontHuge, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2 - 100, COLOR_WHITE, "\uE150", ALIGNED_CENTER);

  Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2, COLOR_WHITE, "EdiZon detected that you're running the SX OS custom firmware. Please note that this\nmay cause unexpected failures, corruption of save data or backups, the Editor failing\nto load save files or configs and many other issues. For your own safety and the\n safety of your Nintendo Switch, please use Atmosphère instead.", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2 + 200 , COLOR_WHITE, "If you want to proceed anyways, press \uE0E0. Otherwise press \uE0EF to exit.", ALIGNED_CENTER);



  Gui::endDraw();
}

void GuiTXWarning::onInput(u32 kdown) {
  if (kdown & KEY_A)
    Gui::g_nextGui = GUI_MAIN;
}

void GuiTXWarning::onTouch(touchPosition &touch) {

}

void GuiTXWarning::onGesture(touchPosition &startPosition, touchPosition &endPosition) {

}
