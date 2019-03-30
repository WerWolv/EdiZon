#include "ui_elements/message_box.hpp"

#include <math.h>

#include "guis/gui.hpp"

MessageBox::MessageBox(std::string message, MessageBox::MessageBoxOptions options) : m_message(message), m_options(options) {
  m_selectedOption = 0;
  m_progress = -1;

  m_selectionAction = [](s8 selection){};
  m_customDraw = [](Gui *gui, s16 x, s16 y){};
}

MessageBox::~MessageBox() {

}

MessageBox* MessageBox::setSelectionAction(std::function<void(s8)> selectionAction) {
  m_selectionAction = selectionAction;

  return this;
}

MessageBox* MessageBox::setCustomDraw(std::function<void(Gui *gui, s16 x, s16 y)> customDraw) {
  m_customDraw = customDraw;

  return this;
}


void MessageBox::draw(Gui *gui) {
  gui->drawRectangled(0, 0, Gui::g_framebuffer_width, Gui::g_framebuffer_height, gui->makeColor(0x00, 0x00, 0x00, 0xA0));
  gui->drawRectangle(250, 180, Gui::g_framebuffer_width - 500, Gui::g_framebuffer_height - 360, currTheme.backgroundColor);

  gui->drawTextAligned(font20, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2 - 75, currTheme.textColor, m_message.c_str(), ALIGNED_CENTER);

  if (m_options == MessageBox::OKAY) {
    gui->drawRectangle(250, Gui::g_framebuffer_height - 260, Gui::g_framebuffer_width - 500, 80, currTheme.selectedButtonColor);
    gui->drawRectangled(245, Gui::g_framebuffer_height - 265, Gui::g_framebuffer_width - 490, 90, currTheme.highlightColor);
    gui->drawRectangle(250, Gui::g_framebuffer_height - 260, Gui::g_framebuffer_width - 500, 80, currTheme.selectedButtonColor);
    gui->drawTextAligned(font20, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height - 237, currTheme.selectedColor, "Okay", ALIGNED_CENTER);
  }
  else if (m_options == MessageBox::YES_NO) {
    gui->drawRectangle(250, Gui::g_framebuffer_height - 260, Gui::g_framebuffer_width - 500, 80, currTheme.selectedButtonColor);
    gui->drawRectangled(245 + ((Gui::g_framebuffer_width - 490) / 2) * m_selectedOption, Gui::g_framebuffer_height - 265, (Gui::g_framebuffer_width - 490) / 2, 90, currTheme.highlightColor);
    gui->drawRectangle(250, Gui::g_framebuffer_height - 260, (Gui::g_framebuffer_width - 500) / 2 - 6, 80, currTheme.selectedButtonColor);
    gui->drawRectangle(250 + ((Gui::g_framebuffer_width - 490) / 2), Gui::g_framebuffer_height - 260, (Gui::g_framebuffer_width - 500) / 2 - 6, 80, currTheme.selectedButtonColor);
    gui->drawTextAligned(font20, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height - 237, currTheme.selectedColor, "No                                            Yes", ALIGNED_CENTER);
  }

  if (m_progress >= 0) {
    gui->drawRectangle(350, Gui::g_framebuffer_height - 300, Gui::g_framebuffer_width - 700, 10, gui->makeColor(0x00, 0x00, 0x00, 0xFF));
    gui->drawRectangle(350, Gui::g_framebuffer_height - 300, ((Gui::g_framebuffer_width - 700) / 100.0F) * m_progress, 10, currTheme.selectedColor);
  }

  m_customDraw(gui, 250, 180);

}

void MessageBox::onInput(u32 kdown) {
  if (m_options == MessageBox::OKAY) {
    if (kdown & KEY_A || kdown & KEY_B) {
      this->hide();
    }
  }
  else if (m_options == MessageBox::YES_NO) {
    if (kdown & KEY_LEFT)
      m_selectedOption = fmax(0, m_selectedOption - 1);

    if (kdown & KEY_RIGHT)
      m_selectedOption = fmin(1, m_selectedOption + 1);

    if (kdown & KEY_A) {
      m_selectionAction(m_selectedOption);
    }

    if (kdown & KEY_B) {
      m_selectionAction(0);
      this->hide();
    }
  }
}

void MessageBox::onTouch(touchPosition &touch) {
  if (m_options == MessageBox::OKAY) {
    if (touch.px > 250 && touch.py > Gui::g_framebuffer_height - 260 && touch.px < Gui::g_framebuffer_width - 250 && touch.py < Gui::g_framebuffer_height - 180) {
      m_selectionAction(0);
      this->hide();
    }
  } else if (m_options == MessageBox::YES_NO) {
    if (touch.px > 250 && touch.py > Gui::g_framebuffer_height - 260 && touch.px < 250 + (Gui::g_framebuffer_width - 500) / 2 && touch.py < Gui::g_framebuffer_height - 180) {
      m_selectionAction(0);
      this->hide();
    } else if (touch.px > 250 + (Gui::g_framebuffer_width - 500) / 2 && touch.py > Gui::g_framebuffer_height - 260 && touch.px < Gui::g_framebuffer_width - 250 && touch.py < Gui::g_framebuffer_height - 180) {
      m_selectionAction(1);
      this->hide();
    }
  }
}

void MessageBox::onGesture(touchPosition startPosition, touchPosition endPosition, bool finish) {

}

void MessageBox::setProgress(s8 progress) {
  this->m_progress = std::min((s8)100, progress);
  requestDraw();
}


void MessageBox::show() {
  if (Gui::g_currMessageBox != nullptr)
    delete Gui::g_currMessageBox;

  Gui::g_currMessageBox = this;
}

void MessageBox::hide() {
  Gui::g_currMessageBox = nullptr;
}
