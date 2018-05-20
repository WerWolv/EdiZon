#include "gui.hpp"

Gui *Gui::g_nextGui = nullptr;

Gui::Gui() {
  this->m_framebuffer = gfxGetFramebuffer(&this->m_framebuffer_width, &this->m_framebuffer_height);
}

Gui::~Gui() {

}

inline u8 Gui::blendColor(u32 src, u32 dst, u8 alpha) {
    u8 one_minus_alpha = (u8)255 - alpha;
    return (dst*alpha + src*one_minus_alpha)/(u8)255;
}

color_t Gui::makeColor(u8 r, u8 g, u8 b, u8 a) {
    color_t clr;
    clr.r = r;
    clr.g = g;
    clr.b = b;
    clr.a = a;
    return clr;
}

inline void Gui::drawPixel(u32 x, u32 y, color_t clr) {
    if (x >= 1280 || y >= 720)
        return;
    u32 off = (y * this->m_framebuffer_width + x)*4;
    this->m_framebuffer[off] = blendColor(this->m_framebuffer[off], clr.r, clr.a); off++;
    this->m_framebuffer[off] = blendColor(this->m_framebuffer[off], clr.g, clr.a); off++;
    this->m_framebuffer[off] = blendColor(this->m_framebuffer[off], clr.b, clr.a); off++;
    this->m_framebuffer[off] = 0xff;
}

inline void Gui::draw4PixelsRaw(u32 x, u32 y, color_t clr) {
    if (x >= 1280 || y >= 720 || x > 1280-4)
        return;

    u32 color = clr.r | (clr.g<<8) | (clr.b<<16) | (0xff<<24);
    u128 val = color | ((u128)color<<32) | ((u128)color<<64) | ((u128)color<<96);
    u32 off = (y * this->m_framebuffer_width + x)*4;
    *((u128*)&this->m_framebuffer[off]) = val;
}

inline const ffnt_page_t* Gui::fontGetPage(const ffnt_header_t* font, u32 page_id) {
    if (page_id >= font->npages)
        return NULL;
    ffnt_pageentry_t* ent = &((ffnt_pageentry_t*)(font+1))[page_id];
    if (ent->size == 0)
        return NULL;
    return (const ffnt_page_t*)((const u8*)font + ent->offset);
}

inline bool Gui::fontLoadGlyph(glyph_t* glyph, const ffnt_header_t* font, u32 codepoint) {
    const ffnt_page_t* page = fontGetPage(font, codepoint >> 8);
    if (!page)
        return false;

    codepoint &= 0xFF;
    u32 off = page->hdr.pos[codepoint];
    if (off == ~(u32)0)
        return false;

    glyph->width   = page->hdr.widths[codepoint];
    glyph->height  = page->hdr.heights[codepoint];
    glyph->advance = page->hdr.advances[codepoint];
    glyph->posX    = page->hdr.posX[codepoint];
    glyph->posY    = page->hdr.posY[codepoint];
    glyph->data    = &page->data[off];
    return true;
}

void Gui::drawGlyph(u32 x, u32 y, color_t clr, const glyph_t* glyph)
{
    u32 i, j;
    const u8* data = glyph->data;
    x += glyph->posX;
    y += glyph->posY;
    for (j = 0; j < glyph->height; j ++)
    {
        for (i = 0; i < glyph->width; i ++)
        {
            clr.a = *data++;
            if (!clr.a) continue;
            drawPixel(x+i, y+j, clr);
        }
    }
}

inline u8 Gui::decodeByte(const char** ptr)
{
    u8 c = (u8)**ptr;
    *ptr += 1;
    return c;
}

// UTF-8 code adapted from http://www.json.org/JSON_checker/utf8_decode.c

inline int8_t Gui::decodeUTF8Cont(const char** ptr) {
    int c = decodeByte(ptr);
    return ((c & 0xC0) == 0x80) ? (c & 0x3F) : -1;
}

inline u32 Gui::decodeUTF8(const char** ptr) {
    u32 r;
    u8 c;
    int8_t c1, c2, c3;

    c = decodeByte(ptr);
    if ((c & 0x80) == 0)
        return c;
    if ((c & 0xE0) == 0xC0) {
        c1 = decodeUTF8Cont(ptr);
        if (c1 >= 0) {
            r = ((c & 0x1F) << 6) | c1;
            if (r >= 0x80)
                return r;
        }
    } else if ((c & 0xF0) == 0xE0) {
        c1 = decodeUTF8Cont(ptr);
        if (c1 >= 0) {
            c2 = decodeUTF8Cont(ptr);
            if (c2 >= 0) {
                r = ((c & 0x0F) << 12) | (c1 << 6) | c2;
                if (r >= 0x800 && (r < 0xD800 || r >= 0xE000))
                    return r;
            }
        }
    } else if ((c & 0xF8) == 0xF0) {
        c1 = decodeUTF8Cont(ptr);
        if (c1 >= 0) {
            c2 = decodeUTF8Cont(ptr);
            if (c2 >= 0) {
                c3 = decodeUTF8Cont(ptr);
                if (c3 >= 0) {
                    r = ((c & 0x07) << 18) | (c1 << 12) | (c2 << 6) | c3;
                    if (r >= 0x10000 && r < 0x110000)
                        return r;
                }
            }
        }
    }
    return 0xFFFD;
}

void Gui::drawText_(const ffnt_header_t* font, u32 x, u32 y, color_t clr, const char* text, u32 max_width) {
    y += font->baseline;
    u32 origX = x;

    while (*text) {
        if (max_width && x-origX >= max_width) {
            break;
        }

        glyph_t glyph;
        u32 codepoint = decodeUTF8(&text);

        if (codepoint == '\n') {
            if (max_width) {
                break;
            }

            x = origX;
            y += font->height;
            continue;
        }

        if (!fontLoadGlyph(&glyph, font, codepoint)) {
            if (!fontLoadGlyph(&glyph, font, '?'))
                continue;
        }

        drawGlyph(x, y, clr, &glyph);
        x += glyph.advance;
    }
}

void Gui::drawText(const ffnt_header_t* font, u32 x, u32 y, color_t clr, const char* text) {
    drawText_(font, x, y, clr, text, 0);
}

void Gui::drawTextTruncate(const ffnt_header_t* font, u32 x, u32 y, color_t clr, const char* text, u32 max_width) {
    drawText_(font, x, y, clr, text, max_width);
}

void Gui::getTextDimensions(const ffnt_header_t* font, const char* text, u32* width_out, u32* height_out) {
    u32 x = 0;
    u32 width = 0, height = 0;
    while (*text) {
        glyph_t glyph;
        u32 codepoint = decodeUTF8(&text);

        if (codepoint == '\n') {
            x = 0;
            height += font->height;
            continue;
        }

        if (!fontLoadGlyph(&glyph, font, codepoint)) {
            if (!fontLoadGlyph(&glyph, font, '?'))
                continue;
        }

        x += glyph.advance;

        if (x > width)
            width = x;
    }

    if (width_out)
        *width_out = width;
    if (height_out)
        *height_out = height;
}

void Gui::drawImage(int x, int y, int width, int height, const u8 *image, ImageMode mode) {
    int tmpx, tmpy;
    int pos;
    color_t current_color;

    for (tmpy = 0; tmpy < height; tmpy++) {
        for (tmpx = 0; tmpx < width; tmpx++) {
            switch (mode) {
                case IMAGE_MODE_RGB24:
                    pos = ((tmpy*width) + tmpx) * 3;
                    current_color = makeColor(image[pos+0], image[pos+1], image[pos+2], 255);
                    break;
                case IMAGE_MODE_RGBA32:
                    pos = ((tmpy*width) + tmpx) * 4;
                    current_color = makeColor(image[pos+0], image[pos+1], image[pos+2], image[pos+3]);
                    break;
            }
            drawPixel(x+tmpx, y+tmpy, current_color);
        }
    }
}

void Gui::drawRectangled(u32 x, u32 y, u32 w, u32 h, color_t color) {
    for (u32 j = y; j < y + h; j++) {
        for (u32 i = x; i < x + w; i++) {
            drawPixel(i, j, color);
        }
    }
}

void Gui::drawRectangle(u32 x, u32 y, u32 w, u32 h, color_t color) {
    for (u32 j = y; j < y + h; j++) {
        for (u32 i = x; i < x + w; i+=4) {
            draw4PixelsRaw(i, j, color);
        }
    }
}

void Gui::drawShadow(u16 x, u16 y, u16 width, u16 height) {
  color_t shadowColor;
  u8 shadowAlphaBase = 80;
  u8 shadowSize = 4;
  u8 shadowInset;

  y += height;

  for(int16_t tmpx = x; tmpx < (x + width); tmpx+=4) {
    for(int16_t tmpy = y; tmpy < (y + height); tmpy++) {
      shadowColor = makeColor(0, 0, 0, shadowAlphaBase * (1.0F - (float)(tmpy - y) / ((float)shadowSize)));
      shadowInset = (tmpy - y);

      if(tmpx >= (x + shadowInset) && tmpx < (x + width - shadowInset))
        for(int16_t i = 0; i < 4; i++) {
          drawPixel(tmpx + i,tmpy, shadowColor);
        }
    }
  }
}
