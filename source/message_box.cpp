#include "message_box.hpp"

#include "gui.hpp"

MessageBox::MessageBox(std::string title, std::string message, MessageBoxOptions options, MessageBoxStyle style) : m_title(title), m_message(message), m_options(options), m_style(style) {

}

MessageBox::~MessageBox() {

}

void MessageBox::draw(Gui *gui) {

}

void MessageBox::onInput(u32 kdown) {

}

void MessageBox::onTouch(touchPosition &touch) {

}

void MessageBox::show() {
  Gui::g_currMessageBox = this;
}

void MessageBox::hide() {
  Gui::g_currMessageBox = nullptr;
}
