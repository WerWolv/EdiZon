#pragma once

#include "gui.hpp"

#include <vector>
#include <unordered_map>

class GuiMain : public Gui {
public:
  GuiMain();
  ~GuiMain();

  void draw();
  void onInput(u32 kdown);

private:
  u8 m_selectedTitle;
  u8 m_selectedAccount;
  u8 m_selectedSaveFile;

  u64 m_selectedTitleId;
  u128 m_selectedUserId;
};
