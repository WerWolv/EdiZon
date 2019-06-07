#pragma once

#include <edizon.h>
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
  color_t textColor;
  color_t backgroundColor;
  color_t highlightColor;
  color_t selectedColor;
  color_t separatorColor;
  color_t selectedButtonColor;
  color_t tooltipColor;
  color_t tooltipTextColor;
} theme_t;

extern theme_t currTheme;

void setTheme(ColorSetId colorSetId);

#ifdef __cplusplus
}
#endif