#include "list_selector.hpp"

#include "gui.hpp"

ListSelector::ListSelector(Gui *m_gui, std::string title, std::string options, std::vector<std::string> &listItems) : m_gui(m_gui), m_title(title), m_options(options), m_listItems(listItems) {
  m_optionsWidth = 0;
  m_optionsHeight = 0;
  selectedItem = 0;

  m_gui->getTextDimensions(font20, const_cast<const char*>(m_options.c_str()), &m_optionsWidth, &m_optionsHeight);
}

ListSelector::~ListSelector() {

}

void ListSelector::draw() {
  m_gui->drawRectangled(0, 0, m_gui->framebuffer_width, 220, m_gui->makeColor(0x00, 0x00, 0x00, 0x80));
  m_gui->drawRectangle(0, 220, m_gui->framebuffer_width, m_gui->framebuffer_height - 120, currTheme.backgroundColor);
  m_gui->drawRectangle(50, 300, m_gui->framebuffer_width - 100, 2, currTheme.textColor);
  m_gui->drawText(font24, 100, 240, currTheme.textColor, m_title.c_str());

  if(m_listItems.size() != 0) {
    for(s16 currItem = -2; currItem < 3; currItem++) {
       if((currItem + selectedItem) >= 0 && (currItem + selectedItem) < (s16)m_listItems.size()) {
         m_gui->drawText(font20, 300, 340 + 60 * (currItem + 2), currTheme.textColor, m_listItems[(currItem + selectedItem)].c_str());
         m_gui->drawRectangle(250, 325 + 60 * (currItem + 2), m_gui->framebuffer_width - 500, 1, currTheme.separatorColor);
         m_gui->drawRectangle(250, 325 + 60 * (currItem + 3), m_gui->framebuffer_width - 500, 1, currTheme.separatorColor);
       }
    }
    m_gui->drawRectangled(245, 320 + 60 * 2, m_gui->framebuffer_width - 490, 71, currTheme.highlightColor);
    m_gui->drawRectangle(250, 325 + 60 * 2, m_gui->framebuffer_width - 500, 61, currTheme.selectedButtonColor);
    m_gui->drawText(font20, 300, 340 + 60 * 2, currTheme.textColor, m_listItems[selectedItem].c_str());
    m_gui->drawShadow(245, 320 + 60 * 2, m_gui->framebuffer_width - 491, 71);
  } else m_gui->drawText(font20, 300, 340 + 60 * 2, currTheme.textColor, "No items present!");

  m_gui->drawRectangle(50, m_gui->framebuffer_height - 70, m_gui->framebuffer_width - 100, 2, currTheme.textColor);
  m_gui->drawText(font20, m_gui->framebuffer_width - m_optionsWidth - 100, m_gui->framebuffer_height - 50, currTheme.textColor, m_options.c_str());
}

ListSelector* ListSelector::setInputAction(std::function<void(u32, u16)> inputActions) {
  m_inputActions = inputActions;

  return this;
}

void ListSelector::onInput(u32 kdown) {
 if(kdown & KEY_B)
   hide();

 if(kdown & KEY_UP)
   if(selectedItem > 0)
     selectedItem--;

 if(kdown & KEY_DOWN)
   if(selectedItem < ((s16)m_listItems.size() - 1))
     selectedItem++;

  m_inputActions(kdown, selectedItem);

}

void ListSelector::onTouch(touchPosition &touch) {

}

void ListSelector::show() {
  m_gui->currListSelector = this;
}

void ListSelector::hide() {
  m_gui->currListSelector = nullptr;
}
