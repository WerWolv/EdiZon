#include "theme.h"

theme_t currTheme;

void setTheme(ColorSetId colorSetId, u32 accentColorA, u32 accentColorB) {
  switch(colorSetId) {
    case ColorSetId_Light:
      currTheme = (theme_t) {
        .textColor = RGBA8(0x00, 0x00, 0x00, 0xFF),
        .backgroundColor = RGBA8(0xE9, 0xEC, 0xF1, 0xFF),
        .highlightColor = RGBA8(0x5B, 0xED, 0xF4, 0x00),
        .separatorColor = RGBA8(0xC9, 0xCC, 0xD1, 0xFF)
      };
      break;
    case ColorSetId_Dark:
      currTheme = (theme_t) {
        .textColor = RGBA8(0xFF, 0xFF, 0xFF, 0xFF),
        .backgroundColor = RGBA8(0x2D, 0x2D, 0x32, 0xFF),
        .highlightColor = RGBA8(0x5B, 0xED, 0xF4, 0x00),
        .separatorColor = RGBA8(0x4D, 0x4D, 0x52, 0xFF)
      };
      break;
  }

  currTheme.accentColorA.color_abgr = accentColorA;
  currTheme.accentColorB.color_abgr = accentColorB;

}
