#pragma once

#include <switch.h>
#include "types.h"

typedef struct {
  color_t textColor;
  color_t backgroundColor;
  color_t highlightColor;
  color_t separatorColor;
  color_t activeColor;

  color_t accentColorA;
  color_t accentColorB;
} theme_t;

extern theme_t currTheme;

void setTheme(ColorSetId colorSetId, uint32_t accentColorA, uint32_t accentColorB);
