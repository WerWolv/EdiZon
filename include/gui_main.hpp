#include "gui.hpp"

#include <vector>

class GuiMain : public Gui {
public:
  GuiMain();
  ~GuiMain();

  void draw();

private:
  uint8_t m_selectedItem;
};
