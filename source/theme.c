#include "theme.h"

theme_t currTheme;

void setTheme(ColorSetId colorSetId, uint32_t accentColorA, uint32_t accentColorB) {
  switch(colorSetId) {
    case ColorSetId_Light:
      currTheme = (theme_t) {
        .textColor = RGBA8(0x00, 0x00, 0x00, 0xFF),
        .backgroundColor = RGBA8(0xE9, 0xEC, 0xF1, 0xFF),
        .highlightColor = RGBA8(0x5B, 0xED, 0xE0, 0xFF),
        .separatorColor = RGBA8(0xDB, 0xDA, 0xDB, 0xFF)
      };
      break;
    case ColorSetId_Dark:
      currTheme = (theme_t) {
        .textColor = RGBA8(0xFF, 0xFF, 0xFF, 0xFF),
        .backgroundColor = RGBA8(0x2D, 0x2D, 0x32, 0xFF),
        .highlightColor = RGBA8(0x5B, 0xED, 0xE0, 0xFF),
        .separatorColor = RGBA8(0xDB, 0xDA, 0xDB, 0xFF)
      };
      break;
  }

  currTheme.accentColorA.color_abgr = accentColorA;
  currTheme.accentColorB.color_abgr = accentColorB;

}
