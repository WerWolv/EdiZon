#pragma once

#include <switch.h>
#include <cstring>

#include "title.hpp"
#include "snackbar.hpp"
#include "list_selector.hpp"

extern "C" {
  #include "theme.h"
  #include "types.h"
}

extern const ffnt_header_t tahoma24_nxfnt;
extern const ffnt_header_t interuiregular20_nxfnt;
extern const ffnt_header_t interuiregular14_nxfnt;
#define font24 &tahoma24_nxfnt
#define font20 &interuiregular20_nxfnt
#define font14 &interuiregular14_nxfnt

enum gui_t {
  GUI_INVALID,
  GUI_MAIN,
  GUI_EDITOR
};

class Gui {
public:
  static enum gui_t g_nextGui;

  u8 *framebuffer;
  u32 framebuffer_width;
  u32 framebuffer_height;

  Snackbar *currSnackbar;
  ListSelector *currListSelector;

  Gui();
  virtual ~Gui();
  virtual void draw() = 0;
  virtual void onInput(u32 kdown) = 0;
  virtual void onTouch(touchPosition &touch) = 0;

  static void resizeImage(u8* in, u8* out, size_t src_width, size_t src_height, size_t dest_width, size_t dest_height);
  inline u8 blendColor(u32 src, u32 dst, u8 alpha);
  color_t makeColor(u8 r, u8 g, u8 b, u8 a);
  void drawRectangle(s16 x, s16 y, s16 w, s16 h, color_t color);
  void drawRectangled(s16 x, s16 y, s16 w, s16 h, color_t color);
  void drawText(const ffnt_header_t* font, s16 x, s16 y, color_t clr, const char* text);
  void drawTextAligned(const ffnt_header_t* font, s16 x, s16 y, color_t clr, const char* text, TextAlignment alignment);
  void drawTextTruncate(const ffnt_header_t* font, s16 x, s16 y, color_t clr, const char* text, u32 max_width);
  void getTextDimensions(const ffnt_header_t* font, const char* text, u32* width_out, u32* height_out);
  void drawImage(s16 x, s16 y, s16 width, s16 height, const u8 *image, ImageMode mode);
  void drawShadow(s16 x, s16 y, s16 width, s16 height);

protected:
  void beginDraw();
  void endDraw();

private:
  void drawText_(const ffnt_header_t* font, s16 x, s16 y, color_t clr, const char* text, s32 max_width);
  inline void draw4PixelsRaw(s16 x, s16 y, color_t clr);
  inline bool fontLoadGlyph(glyph_t* glyph, const ffnt_header_t* font, u32 codepoint);
  void drawGlyph(s16 x, s16 y, color_t clr, const glyph_t* glyph);
  inline u8 decodeByte(const char** ptr);
  inline s8 decodeUTF8Cont(const char** ptr);
  inline u32 decodeUTF8(const char** ptr);
  inline const ffnt_page_t* fontGetPage(const ffnt_header_t* font, u32 page_id);
  inline void drawPixel(s16 x, s16 y, color_t clr);
};

#define COLOR_WHITE makeColor(255, 255, 255, 255)
#define COLOR_BLACK makeColor(0, 0, 0, 255)
