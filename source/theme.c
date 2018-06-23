#include "theme.h"

#include "A_dark_bin.h"
#include "A_light_bin.h"
#include "B_dark_bin.h"
#include "B_light_bin.h"
#include "X_dark_bin.h"
#include "X_light_bin.h"
#include "Y_dark_bin.h"
#include "Y_light_bin.h"
#include "L_dark_bin.h"
#include "L_light_bin.h"
#include "R_dark_bin.h"
#include "R_light_bin.h"
#include "plus_dark_bin.h"
#include "plus_light_bin.h"
#include "minus_dark_bin.h"
#include "minus_light_bin.h"

theme_t currTheme;

void setTheme(ColorSetId colorSetId) {
  switch(colorSetId) {
    case ColorSetId_Light:
      currTheme = (theme_t) {
        .textColor            =      RGBA8(0x00, 0x00, 0x00, 0xFF),
        .backgroundColor      =      RGBA8(0xEA, 0xEA, 0xEA, 0xFF),
        .highlightColor       =      RGBA8(0x27, 0xA3, 0xC7, 0xFF),
        .selectedColor        =      RGBA8(0x50, 0x2D, 0xE4, 0xFF),
        .separatorColor       =      RGBA8(0x60, 0x60, 0x60, 0x80),
        .selectedButtonColor  =      RGBA8(0xFD, 0xFD, 0xFD, 0xFF),

        .buttonA              =      A_dark_bin,
        .buttonB              =      B_dark_bin,
        .buttonX              =      X_dark_bin,
        .buttonY              =      Y_dark_bin,
        .buttonL              =      L_dark_bin,
        .buttonR              =      R_dark_bin,
        .buttonPlus           =      plus_dark_bin,
        .buttonMinus          =      minus_dark_bin
      };
      break;
    case ColorSetId_Dark:
      currTheme = (theme_t) {
        .textColor            =      RGBA8(0xFF, 0xFF, 0xFF, 0xFF),
        .backgroundColor      =      RGBA8(0x31, 0x31, 0x31, 0xFF),
        .highlightColor       =      RGBA8(0x27, 0xA3, 0xC7, 0xFF),
        .selectedColor        =      RGBA8(0x59, 0xED, 0xC0, 0xFF),
        .separatorColor       =      RGBA8(0x60, 0x60, 0x60, 0x80),
        .selectedButtonColor  =      RGBA8(0x25, 0x26, 0x2A, 0xFF),

        .buttonA              =      A_light_bin,
        .buttonB              =      B_light_bin,
        .buttonX              =      X_light_bin,
        .buttonY              =      Y_light_bin,
        .buttonL              =      L_light_bin,
        .buttonR              =      R_light_bin,
        .buttonPlus           =      plus_light_bin,
        .buttonMinus          =      minus_light_bin
      };
      break;
  }
}
