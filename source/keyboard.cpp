#include "keyboard.hpp"

#include "gui.hpp"

#include <cmath>

#define KEY_WIDTH 92
#define KEY_HEIGHT 60
#define KEY_SPACING 4

u8 keys[2][4][11] = { {
                    {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-'},
                    {'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '/'},
                    {'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ':', '\''},
                    {'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '?', '!'}
                  },
                  {
                    {'#', '[', ']', '$', '%', '^', '&', '*', '(', ')', '_'},
                    {'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@'},
                    {'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\"'},
                    {'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '+', '='}
                  }
                };

u16 startYOffset = 0, startYOffsetNext = 0;

Keyboard::Keyboard(std::string title, u8 maxInputLength) : m_title(title), m_maxInputLength(maxInputLength) {
  m_enteredString = "";

  m_inputFinishedAction = [](std::string enteredString){};

  startYOffset = 500;
  startYOffsetNext = 0;
}

Keyboard::~Keyboard() {

}

void Keyboard::show() {
  Gui::g_currKeyboard = this;
}

void Keyboard::hide() {
  Gui::g_currKeyboard = nullptr;
}

void Keyboard::update() {
  float deltaOffsetStart = startYOffsetNext - startYOffset;
  float scrollSpeedStart = deltaOffsetStart / 64.0F;

  if (startYOffset != startYOffsetNext) {
    if (startYOffsetNext > startYOffset)
      startYOffset += ceil((abs(deltaOffsetStart) > scrollSpeedStart) ? scrollSpeedStart : deltaOffsetStart);
    else
      startYOffset += floor((abs(deltaOffsetStart) > scrollSpeedStart) ? scrollSpeedStart : deltaOffsetStart);
  }

  if(startYOffset == startYOffsetNext && startYOffset == 500)
    hide();
}

void Keyboard::draw(Gui *gui) {
  gui->drawRectangled(0, 0, Gui::g_framebuffer_width, 220 + startYOffset, gui->makeColor(0x00, 0x00, 0x00, 0x80 * (1 - (startYOffset / 500.0F))));
  gui->drawRectangle(0, 220 + startYOffset, Gui::g_framebuffer_width, Gui::g_framebuffer_height - 120, currTheme.backgroundColor);

  gui->drawRectangle(0, 220 + startYOffset, Gui::g_framebuffer_width, 80, currTheme.backgroundColor);
  gui->drawRectangle(0, Gui::g_framebuffer_height - 70 + startYOffset, Gui::g_framebuffer_width, 70, currTheme.backgroundColor);
  gui->drawRectangle((u32)((Gui::g_framebuffer_width - 1220) / 2), 300 + startYOffset, 1220, 1, currTheme.textColor);

  gui->drawRectangle((u32)((Gui::g_framebuffer_width - 1220) / 2), Gui::g_framebuffer_height - 73 + startYOffset, 1220, 1, currTheme.textColor);
}

void Keyboard::onInput(u32 kdown) {
  if (kdown & KEY_X)
    startYOffsetNext = 500;
}

Keyboard* Keyboard::setInputFinishedAction(std::function<void(std::string)> inputFinishedAction) {
  return this;
}
