#pragma once

#include <string>
#include <functional>
#include <vector>

#include <edizon.h>

class Gui;

class ListSelector {
public:
  ListSelector(std::string title, std::string options, std::vector<std::string> &listItems);
  ~ListSelector();

  ListSelector* setInputAction(std::function<void(u32 kdown, u16 selectedItem)> inputActions);

  void update();
  void draw(Gui *gui);
  void onInput(u32 kdown);
  void onTouch(touchPosition &touch);
  void onGesture(touchPosition startPosition, touchPosition endPosition, bool finish);

  void show();
  void hide();

  u16 selectedItem;

private:
  std::string m_title, m_options;
  std::vector<std::string> &m_listItems;
  std::function<void(u32, u16)> m_inputActions;

  u32 m_optionsWidth, m_optionsHeight;
  bool m_isShown;
  s16 yOffset, yOffsetNext;
  s16 startYOffset, startYOffsetNext;
};
