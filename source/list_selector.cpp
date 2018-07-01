#include "list_selector.hpp"

#include <cmath>

#include "gui.hpp"

ListSelector::ListSelector(Gui *m_gui, std::string title, std::string options, std::vector<std::string> &listItems) : m_gui(m_gui), m_title(title), m_options(options), m_listItems(listItems) {
  m_optionsWidth = 0;
  m_optionsHeight = 0;
  selectedItem = 0;

  yOffsetNext = 0;
  yOffset = 0;

  startYOffset = 500;
  startYOffsetNext = 0;
}

ListSelector::~ListSelector() {

}

void ListSelector::draw() {
  float deltaOffset = yOffsetNext - yOffset;
  float scrollSpeed = deltaOffset / 4.0F;
  float deltaOffsetStart = startYOffsetNext - startYOffset;
  float scrollSpeedStart = deltaOffsetStart / 4.0F;

  m_gui->drawRectangled(0, 0, m_gui->framebuffer_width, 220 + startYOffset, m_gui->makeColor(0x00, 0x00, 0x00, 0x80 * (1 - (startYOffset / 500.0F))));
  m_gui->drawRectangle(0, 220 + startYOffset, m_gui->framebuffer_width, m_gui->framebuffer_height - 120, currTheme.backgroundColor);

  m_gui->drawRectangled(220, 305 + 60 * 2 + deltaOffset - 5 + startYOffset, m_gui->framebuffer_width - 477, 71, currTheme.highlightColor);
  m_gui->drawRectangle(225, 305 + 60 * 2 + deltaOffset + startYOffset, m_gui->framebuffer_width - 490, 61, currTheme.selectedButtonColor);

  yOffsetNext = 60 * selectedItem;
  if (m_listItems.size() != 0) {
    for (s8 currItem = 0; currItem < m_listItems.size(); currItem++) {
      m_gui->drawText(font20, 270, fmax(440 + 60 * currItem - yOffset, 220) + startYOffset, currTheme.textColor, m_listItems[currItem].c_str());
    }

    m_gui->drawRectangle(0, 220 + startYOffset, m_gui->framebuffer_width, 80, currTheme.backgroundColor);
    m_gui->drawRectangle(0, m_gui->framebuffer_height - 70 + startYOffset, m_gui->framebuffer_width, 70, currTheme.backgroundColor);
    m_gui->drawRectangle((u32)((m_gui->framebuffer_width - 1220) / 2), 300 + startYOffset, 1220, 1, currTheme.textColor);
    m_gui->drawText(font24, 100, 240 + startYOffset, currTheme.textColor, m_title.c_str());

  }
  else
    m_gui->drawText(font20, 300, 340 + 60 * 2 + startYOffset, currTheme.textColor, "No items present!");

  m_gui->drawRectangle((u32)((m_gui->framebuffer_width - 1220) / 2), m_gui->framebuffer_height - 73 + startYOffset, 1220, 1, currTheme.textColor);
  m_gui->drawTextAligned(font20, m_gui->framebuffer_width - 100, m_gui->framebuffer_height - 50 + startYOffset, currTheme.textColor, m_options.c_str(), ALIGNED_RIGHT);

  if (yOffset != yOffsetNext) {
    if (yOffsetNext > yOffset)
      yOffset += ceil((abs(deltaOffset) > scrollSpeed) ? scrollSpeed : deltaOffset);
    else
      yOffset += floor((abs(deltaOffset) > scrollSpeed) ? scrollSpeed : deltaOffset);
  }

  if (startYOffset != startYOffsetNext) {
    if (startYOffsetNext > startYOffset)
      startYOffset += ceil((abs(deltaOffsetStart) > scrollSpeedStart) ? scrollSpeedStart : deltaOffsetStart);
    else
      startYOffset += floor((abs(deltaOffsetStart) > scrollSpeedStart) ? scrollSpeedStart : deltaOffsetStart);
  }

  if(startYOffset == startYOffsetNext && startYOffset == 500)
    hide();

}

ListSelector* ListSelector::setInputAction(std::function<void(u32, u16)> inputActions) {
  m_inputActions = inputActions;

  return this;
}

void ListSelector::onInput(u32 kdown) {
 if (kdown & KEY_B)
   startYOffsetNext = 500;

 if (kdown & KEY_UP)
   if (selectedItem > 0)
     selectedItem--;

 if (kdown & KEY_DOWN)
   if (selectedItem < (static_cast<s16>(m_listItems.size() - 1)))
     selectedItem++;

  m_inputActions(kdown, selectedItem);

}

void ListSelector::onTouch(touchPosition &touch) {
  if(touch.px > 250 && touch.px < m_gui->framebuffer_width - 250) {
    if(touch.py > 325 && touch.py < (325 + 60 * 5)) {
      s8 touchPos = ((touch.py - 325) / 60.0F); //325 + 60 * (currItem + 2)

      if((selectedItem + touchPos - 2) >= 0 && (selectedItem + touchPos - 2) <= (static_cast<s16>(m_listItems.size() - 1)))
        selectedItem += (touchPos - 2);
    }
  }
}

void ListSelector::show() {
  m_gui->currListSelector = this;
}

void ListSelector::hide() {
  m_gui->currListSelector = nullptr;
}
