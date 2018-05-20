#pragma once

#include "gui.hpp"
#include "title.hpp"

#include <unordered_map>

class GuiEditor : public Gui {
public:
  GuiEditor();
  ~GuiEditor();

  void draw();
  void onInput(u32 kdown);

private:
  struct Account {
    AccountProfile profile;
    AccountUserData userData;
    AccountProfileBase profileBase;

    size_t profileImageSize;
    u8 *profileImage;
  };

  std::unordered_map<u128, struct Account> accounts;
};
