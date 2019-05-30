#include "guis/gui_tx_warning.hpp"

#include "helpers/config.hpp"

GuiTXWarning::GuiTXWarning() : Gui() {
  Config::getConfig()->hideSX = false;
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

  Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2, COLOR_WHITE, "EdiZon detected that you're running the 'SX OS' CFW. Please note that this CFW has erroneously\n implemented services that can cause unexpected failures, corruption of save data\n or backups, the editor failing to load save files or configs, RAM editing not being\n supported and other issues. For the safety of your Switch, use a free open\n source CFW instead. \n To continue anyway press \uE0E0, otherwise press \uE0E1 to exit.", ALIGNED_CENTER);

  Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2 + 250, COLOR_WHITE, "\uE070  Don't show this warning anymore", ALIGNED_CENTER);

  if (!Config::getConfig()->hideSX)
    Gui::drawRectangle(Gui::g_framebuffer_width / 2 - 228, Gui::g_framebuffer_height / 2 + 258, 14, 16, Gui::makeColor(0xC5, 0x39, 0x29, 0xFF));

  Gui::endDraw();
}

void GuiTXWarning::onInput(u32 kdown) {
  if (kdown & KEY_B)
    Gui::g_requestExit = true;

  if (kdown & KEY_A)
    Gui::g_nextGui = GUI_MAIN;
}

void GuiTXWarning::onTouch(touchPosition &touch) {
  if (touch.px > 400 && touch.px < 900 && touch.py > 600 && touch.py < 660) {
    Config::getConfig()->hideSX = !Config::getConfig()->hideSX;
    Config::writeConfig();
  }
}

void GuiTXWarning::onGesture(touchPosition startPosition, touchPosition endPosition, bool finish) {

}
