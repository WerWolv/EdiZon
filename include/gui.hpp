#pragma once

#include <switch.h>
#include <string.h>

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

class Gui {
public:
  Gui();
  virtual ~Gui();
  virtual void draw() = 0;

protected:
  uint8_t *m_framebuffer;
  uint32_t m_framebuffer_width;
  uint32_t m_framebuffer_height;


  inline bool fontLoadGlyph(glyph_t* glyph, const ffnt_header_t* font, uint32_t codepoint);
  void drawGlyph(uint32_t x, uint32_t y, color_t clr, const glyph_t* glyph);
  inline uint8_t decodeByte(const char** ptr);
  inline int8_t decodeUTF8Cont(const char** ptr);
  inline uint32_t decodeUTF8(const char** ptr);
  void drawText_(const ffnt_header_t* font, uint32_t x, uint32_t y, color_t clr, const char* text, uint32_t max_width);
  inline const ffnt_page_t* fontGetPage(const ffnt_header_t* font, uint32_t page_id);
  inline uint8_t blendColor(uint32_t src, uint32_t dst, uint8_t alpha);
  inline color_t makeColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
  inline void draw4PixelsRaw(uint32_t x, uint32_t y, color_t clr);
  void drawRectangle(uint32_t x, uint32_t y, uint32_t w, uint32_t h, color_t color);
  void drawRectangled(uint32_t x, uint32_t y, uint32_t w, uint32_t h, color_t color);
  inline void drawPixel(uint32_t x, uint32_t y, color_t clr);
  void drawText(const ffnt_header_t* font, uint32_t x, uint32_t y, color_t clr, const char* text);
  void drawTextTruncate(const ffnt_header_t* font, uint32_t x, uint32_t y, color_t clr, const char* text, uint32_t max_width);
  void getTextDimensions(const ffnt_header_t* font, const char* text, uint32_t* width_out, uint32_t* height_out);
  void drawImage(int x, int y, int width, int height, const uint8_t *image, ImageMode mode);
};
