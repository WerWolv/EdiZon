#pragma once

#include <switch.h>
#include <cstring>

#include "title.hpp"

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
  static Title *g_currTitle;

  Gui();
  virtual ~Gui();
  virtual void draw() = 0;
  virtual void onInput(u32 kdown) = 0;


protected:
  u8 *m_framebuffer;
  u32 m_framebuffer_width;
  u32 m_framebuffer_height;

  inline u8 blendColor(u32 src, u32 dst, u8 alpha);
  color_t makeColor(u8 r, u8 g, u8 b, u8 a);
  void drawRectangle(u32 x, u32 y, u32 w, u32 h, color_t color);
  void drawRectangled(u32 x, u32 y, u32 w, u32 h, color_t color);
  void drawText(const ffnt_header_t* font, u32 x, u32 y, color_t clr, const char* text);
  void drawTextTruncate(const ffnt_header_t* font, u32 x, u32 y, color_t clr, const char* text, u32 max_width);
  void getTextDimensions(const ffnt_header_t* font, const char* text, u32* width_out, u32* height_out);
  void drawImage(int x, int y, int width, int height, const u8 *image, ImageMode mode);
  void drawShadow(u16 x, u16 y, u16 width, u16 height);

private:
  void drawText_(const ffnt_header_t* font, u32 x, u32 y, color_t clr, const char* text, u32 max_width);
  inline void draw4PixelsRaw(u32 x, u32 y, color_t clr);
  inline bool fontLoadGlyph(glyph_t* glyph, const ffnt_header_t* font, u32 codepoint);
  void drawGlyph(u32 x, u32 y, color_t clr, const glyph_t* glyph);
  inline u8 decodeByte(const char** ptr);
  inline int8_t decodeUTF8Cont(const char** ptr);
  inline u32 decodeUTF8(const char** ptr);
  inline const ffnt_page_t* fontGetPage(const ffnt_header_t* font, u32 page_id);
  inline void drawPixel(u32 x, u32 y, color_t clr);
};

#define COLOR_WHITE makeColor(255, 255, 255, 255)
#define COLOR_BLACK makeColor(0, 0, 0, 255)
