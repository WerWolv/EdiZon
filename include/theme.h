#pragma once

#include <switch.h>
#include "types.h"

typedef struct {
  color_t textColor;
  color_t backgroundColor;
  color_t highlightColor;
  color_t selectedColor;
  color_t separatorColor;
  color_t selectedButtonColor;

  const u8 *buttonA;
  const u8 *buttonB;
  const u8 *buttonX;
  const u8 *buttonY;
  const u8 *buttonL;
  const u8 *buttonR;
  const u8 *buttonPlus;
  const u8 *buttonMinus;
} theme_t;

extern theme_t currTheme;

void setTheme(ColorSetId colorSetId);
