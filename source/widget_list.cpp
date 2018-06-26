#include "widget_list.hpp"

#include "list_selector.hpp"

WidgetList::WidgetList(Gui *gui, u8 addressSize, u8 valueSize, std::vector<std::string> listItemNames, std::vector<u64> listItemValues) : Widget(addressSize, valueSize), m_gui(gui), m_listItemNames(listItemNames), m_listItemValues(listItemValues) {

}

WidgetList::~WidgetList() {

}

void WidgetList::draw(Gui *gui, u16 x, u16 y) {
  std::stringstream ss;

  if(std::find(m_listItemValues.begin(), m_listItemValues.end(), Widget::getValue()) != m_listItemValues.end()) {
    ptrdiff_t pos = find(m_listItemValues.begin(), m_listItemValues.end(), Widget::getValue()) - m_listItemValues.begin();
    ss << m_listItemNames[pos] << " [0x" << std::setfill('0') << std::setw(Widget::m_valueSize * 2) << std::uppercase << std::hex << m_listItemValues[pos] << "]";
  } else {
    ss << "Unknown value" << " [0x" << std::setfill('0') << std::setw(Widget::m_valueSize * 2) << std::uppercase << std::hex << Widget::getValue() << "]";
  }

  gui->drawTextAligned(font20, x + WIDGET_WIDTH - 140, y + (WIDGET_HEIGHT / 2.0F), currTheme.selectedColor, ss.str().c_str(), ALIGNED_RIGHT);
}

void WidgetList::onInput(u32 kdown) {
  if (kdown & KEY_A && m_gui->currListSelector == nullptr) {
    (new ListSelector(m_gui, "Choose item", "\x01 - Select      \x02 - Back", m_listItemNames))->setInputAction([&](u32 k, u16 selectedItem){
      if(k & KEY_A) {
        Widget::setValue(m_listItemValues[selectedItem]);
        m_gui->currListSelector->hide();
      }
    })->show();
  }
}

void WidgetList::onTouch(touchPosition &touch) {
  (new ListSelector(m_gui, "Choose item", "\x01 - Select      \x02 - Back", m_listItemNames))->setInputAction([&](u32 k, u16 selectedItem){
    if(k & KEY_A) {
      Widget::setValue(m_listItemValues[selectedItem]);
      m_gui->currListSelector->hide();
    }
  })->show();
}
