#include "gui.hpp"

Gui::Gui() {
  this->m_framebuffer = (uint32_t*) gfxGetFramebuffer(&this->m_framebuffer_width, &this->m_framebuffer_height);
}

Gui::~Gui() {

}

void Gui::draw() {
  for(uint32_t i = 0; i < this->m_framebuffer_width; i++) {
    for(uint32_t j = 0; j < this->m_framebuffer_height; j++) {
      this->m_framebuffer[i + j * this->m_framebuffer_width] = currTheme.backgroundColor.color_abgr;
    }
  }

  drawText(font14, 100, 100, currTheme.textColor, "HELLO WORLD");



  gfxFlushBuffers();
  gfxSwapBuffers();
  gfxWaitForVsync();
}

inline uint8_t Gui::blendColor(uint32_t src, uint32_t dst, uint8_t alpha) {
    uint8_t one_minus_alpha = (uint8_t)255 - alpha;
    return (dst*alpha + src*one_minus_alpha)/(uint8_t)255;
}

inline color_t Gui::makeColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    color_t clr;
    clr.r = r;
    clr.g = g;
    clr.b = b;
    clr.a = a;
    return clr;
}

inline void Gui::drawPixel(uint32_t x, uint32_t y, color_t clr) {
    if (x >= 1280 || y >= 720)
        return;
    uint32_t off = (y * this->m_framebuffer_width + x);
    this->m_framebuffer[off] = blendColor(this->m_framebuffer[off], clr.r, clr.a); off++;
    this->m_framebuffer[off] = blendColor(this->m_framebuffer[off], clr.g, clr.a); off++;
    this->m_framebuffer[off] = blendColor(this->m_framebuffer[off], clr.b, clr.a); off++;
    this->m_framebuffer[off] = 0xff;
}

inline void Gui::draw4PixelsRaw(uint32_t x, uint32_t y, color_t clr) {
    if (x >= 1280 || y >= 720 || x > 1280-4)
        return;

    uint32_t color = clr.r | (clr.g<<8) | (clr.b<<16) | (0xff<<24);
    u128 val = color | ((u128)color<<32) | ((u128)color<<64) | ((u128)color<<96);
    uint32_t off = (y * this->m_framebuffer_width + x)*4;
    *((u128*)&this->m_framebuffer[off]) = val;
}

inline const ffnt_page_t* Gui::fontGetPage(const ffnt_header_t* font, uint32_t page_id) {
    if (page_id >= font->npages)
        return NULL;
    ffnt_pageentry_t* ent = &((ffnt_pageentry_t*)(font+1))[page_id];
    if (ent->size == 0)
        return NULL;
    return (const ffnt_page_t*)((const uint8_t*)font + ent->offset);
}

inline bool Gui::fontLoadGlyph(glyph_t* glyph, const ffnt_header_t* font, uint32_t codepoint) {
    const ffnt_page_t* page = fontGetPage(font, codepoint >> 8);
    if (!page)
        return false;

    codepoint &= 0xFF;
    uint32_t off = page->hdr.pos[codepoint];
    if (off == ~(uint32_t)0)
        return false;

    glyph->width   = page->hdr.widths[codepoint];
    glyph->height  = page->hdr.heights[codepoint];
    glyph->advance = page->hdr.advances[codepoint];
    glyph->posX    = page->hdr.posX[codepoint];
    glyph->posY    = page->hdr.posY[codepoint];
    glyph->data    = &page->data[off];
    return true;
}

void Gui::drawGlyph(uint32_t x, uint32_t y, color_t clr, const glyph_t* glyph)
{
    uint32_t i, j;
    const uint8_t* data = glyph->data;
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

inline uint8_t Gui::decodeByte(const char** ptr)
{
    uint8_t c = (uint8_t)**ptr;
    *ptr += 1;
    return c;
}

// UTF-8 code adapted from http://www.json.org/JSON_checker/utf8_decode.c

inline int8_t Gui::decodeUTF8Cont(const char** ptr) {
    int c = decodeByte(ptr);
    return ((c & 0xC0) == 0x80) ? (c & 0x3F) : -1;
}

inline uint32_t Gui::decodeUTF8(const char** ptr) {
    uint32_t r;
    uint8_t c;
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

void Gui::drawText_(const ffnt_header_t* font, uint32_t x, uint32_t y, color_t clr, const char* text, uint32_t max_width) {
    y += font->baseline;
    uint32_t origX = x;

    while (*text) {
        if (max_width && x-origX >= max_width) {
            break;
        }

        glyph_t glyph;
        uint32_t codepoint = decodeUTF8(&text);

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

void Gui::drawText(const ffnt_header_t* font, uint32_t x, uint32_t y, color_t clr, const char* text) {
    drawText_(font, x, y, clr, text, 0);
}

void Gui::drawTextTruncate(const ffnt_header_t* font, uint32_t x, uint32_t y, color_t clr, const char* text, uint32_t max_width) {
    drawText_(font, x, y, clr, text, max_width);
}

void Gui::getTextDimensions(const ffnt_header_t* font, const char* text, uint32_t* width_out, uint32_t* height_out) {
    uint32_t x = 0;
    uint32_t width = 0, height = 0;
    while (*text) {
        glyph_t glyph;
        uint32_t codepoint = decodeUTF8(&text);

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

void Gui::drawImage(int x, int y, int width, int height, const uint8_t *image, ImageMode mode) {
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

void Gui::rectangled(uint32_t x, uint32_t y, uint32_t w, uint32_t h, color_t color) {
    for (uint32_t j = y; j < y + h; j++) {
        for (uint32_t i = x; i < x + w; i++) {
            drawPixel(i, j, color);
        }
    }
}

void Gui::rectangle(uint32_t x, uint32_t y, uint32_t w, uint32_t h, color_t color) {
    for (uint32_t j = y; j < y + h; j++) {
        for (uint32_t i = x; i < x + w; i+=4) {
            draw4PixelsRaw(i, j, color);
        }
    }
}
