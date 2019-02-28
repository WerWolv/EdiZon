#include "guis/gui.hpp"

#include <math.h>
#include <functional>

static float menuTimer = 0.0F;
static u32 stride;

Gui::Gui() {
  Gui::g_currSnackbar = nullptr;
  Gui::g_currListSelector = nullptr;
  Gui::g_currMessageBox = nullptr;

  m_fontLibret = 1;
  m_fontFacesTotal = 0;

  if(R_FAILED(plInitialize())) printf("plInitialize failed!\n");

  if(!fontInit())
    printf("Font not initialied!\n");
}

Gui::~Gui() {
  Gui::g_currSnackbar = nullptr;
  Gui::g_currListSelector = nullptr;
  Gui::g_currMessageBox = nullptr;

  fontExit();
  plExit();
}

void Gui::update() {
  menuTimer += 0.005;

  if (Gui::g_currListSelector != nullptr)
    Gui::g_currListSelector->update();

  if (Gui::g_currSnackbar != nullptr) {
    Gui::g_currSnackbar->update();

    if (Gui::g_currSnackbar->isDead()) {
      delete Gui::g_currSnackbar;
      Gui::g_currSnackbar = nullptr;
    }
  }

  float highlightMultiplier = (sin(menuTimer) + 1) / 2.0F;
  currTheme.highlightColor.r = 0x27 + 0x61 * highlightMultiplier;
  currTheme.highlightColor.g = 0xA3 + 0x4F * highlightMultiplier;
  currTheme.highlightColor.b = 0xC7 + 0x29 * highlightMultiplier;
  currTheme.highlightColor.a = 0xC0 + 0x3F * ((highlightMultiplier * 2) - 1);
}

inline u8 Gui::blendColor(u32 src, u32 dst, u8 alpha) {
  u8 one_minus_alpha = static_cast<u8>(255) - alpha;
  return (dst*alpha + src*one_minus_alpha) / static_cast<u8>(255);
}

color_t Gui::makeColor(u8 r, u8 g, u8 b, u8 a) {
  color_t clr;
  clr.r = r;
  clr.g = g;
  clr.b = b;
  clr.a = a;

  return clr;
}

inline void Gui::drawPixel(s16 x, s16 y, color_t clr) {
  if (x >= 1280 || y >= 720 || x < 0 || y < 0)
      return;

  u32 off = y * stride + x * 4;
  framebuffer[off] = blendColor(framebuffer[off], clr.r, clr.a); off++;
  framebuffer[off] = blendColor(framebuffer[off], clr.g, clr.a); off++;
  framebuffer[off] = blendColor(framebuffer[off], clr.b, clr.a); off++;
  framebuffer[off] = 0xFF;
}

inline void Gui::draw4PixelsRaw(s16 x, s16 y, color_t clr) {
  if (x >= 1280 || y >= 720 || x > 1280-4 || x < 0 || y < 0)
      return;

  u32 color = clr.r | (clr.g<<8) | (clr.b<<16) | (0xff<<24);
  u128 val = color | ((u128)color<<32) | ((u128)color<<64) | ((u128)color<<96);
  u32 off = y * stride + x * 4;
  *((u128*)&framebuffer[off]) = val;
}

bool Gui::fontInit() {
  FT_Error ret = 0;

  for (u32 i = 0; i < FONT_FACES_MAX; i++) m_fontFacesRet[i] = 1;

  ret = FT_Init_FreeType(&m_fontLibrary);
  m_fontLibret = ret;
  if (m_fontLibret) return false;

  PlFontData fonts[PlSharedFontType_Total];

  if (R_FAILED(plGetSharedFont(0, fonts, FONT_FACES_MAX, &m_fontFacesTotal))) return false;

  for (u32 i = 0; i < m_fontFacesTotal; i++) {
    ret = FT_New_Memory_Face(m_fontLibrary, (FT_Byte*)fonts[i].address, fonts[i].size, 0, &m_fontFaces[i]);

    m_fontFacesRet[i] = ret;
    if (ret) return false;
  }

  return true;
}

void Gui::fontExit() {
  for (u32 i = 0; i < m_fontFacesTotal; i++)
    if (m_fontFacesRet[i] == 0) FT_Done_Face(m_fontFaces[i]);

  if (m_fontLibret == 0) FT_Done_FreeType(m_fontLibrary);
}

bool Gui::setFontType(u32 font) {
  u32 scale = 0;
  FT_Error ret = 0;

  switch (font) {
    case font14: scale = 4; break;
    case font20: scale = 6; break;
    case font24: scale = 7; break;
    case fontHuge: scale = 20; break;
    default: return false;
  }

  for (u32 i = 0; i < m_fontFacesTotal; i++) {
    ret = FT_Set_Char_Size(m_fontFaces[i], 0, scale * 64, 300, 300);

    if (ret) return false;
  }

  return true;
}

inline bool Gui::fontLoadGlyph(glyph_t* glyph, u32 font, u32 codepoint) {
  FT_Face face;
  FT_Error ret = 0;
  FT_GlyphSlot slot;
  FT_UInt glyphIndex;
  FT_Bitmap *bitmap;

  if (m_fontFacesTotal == 0) return false;

  for (u32 i = 0; i < m_fontFacesTotal; i++) {
    face = m_fontFaces[i];
    m_fontLastUsedFace = face;
    glyphIndex = FT_Get_Char_Index(face, codepoint);

    if (glyphIndex == 0) continue;

    ret = FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT);

    if (ret==0)
      ret = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);

    if (ret) return false;

    break;
  }

  slot = face->glyph;
  bitmap = &slot->bitmap;

  glyph->width   = bitmap->width;
  glyph->height  = bitmap->rows;
  glyph->pitch   = bitmap->pitch;
  glyph->data    = bitmap->buffer;
  glyph->advance = slot->advance.x >> 6;
  glyph->posX    = slot->bitmap_left;
  glyph->posY    = slot->bitmap_top;

  return true;
}

void Gui::drawGlyph(s16 x, s16 y, color_t clr, const glyph_t* glyph) {
  const u8* data = glyph->data;
  x += glyph->posX;
  y -= glyph->posY;

  for (u32 j = 0; j < glyph->height; j ++) {
    for (u32 i = 0; i < glyph->width; i ++) {
      color_t pixelColor = clr;
      pixelColor.a = data[i] * (static_cast<float>(clr.a) / 0xFF);
      if (!pixelColor.a) continue;
      drawPixel(x+i, y+j, pixelColor);
    }
    data += glyph->pitch;
  }
}

inline u8 Gui::decodeByte(const char** ptr) {
  u8 c = static_cast<u8>(**ptr);
  (*ptr)++;

  return c;
}

// UTF-8 code adapted from http://www.json.org/JSON_checker/utf8_decode.c

inline s8 Gui::decodeUTF8Cont(const char** ptr) {
  s32 c = decodeByte(ptr);
  return ((c & 0xC0) == 0x80) ? (c & 0x3F) : -1;
}

inline u32 Gui::decodeUTF8(const char** ptr) {
  u32 r;
  u8 c;
  s8 c1, c2, c3;

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

void Gui::drawText_(u32 font, s16 x, s16 y, color_t clr, const char* text, s32 max_width, const char* end_text) {
  u32 origX = x;

  if (m_fontFacesTotal == 0) return;
  if (!setFontType(font)) return;
  m_fontLastUsedFace = m_fontFaces[0];

  switch (font) {
    case font14: y += 20; break;
    case font20: y += 26; break;
    case font24: y += 35; break;
  }

  if (text == nullptr) return;

  while (*text) {
    if (max_width && x - origX >= static_cast<u32>(max_width)) {
      text = end_text;
      max_width = 0;
    }

    glyph_t glyph;
    uint32_t codepoint = decodeUTF8(&text);

    if (codepoint == '\n') {
      if (max_width) {
          text = end_text;
          max_width = 0;
          continue;
      }

      x = origX;

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

void Gui::drawText(u32 font, s16 x, s16 y, color_t clr, const char* text) {
  drawText_(font, x, y, clr, text, 0, nullptr);
}

std::vector<std::string> Gui::split(const std::string& s, const char& c) {
	std::string buff {""};
	std::vector<std::string> v;

	for(auto n:s) {
		if (n != c) buff += n;
    else if (n == c && buff != "") {
       v.push_back(buff);
        buff = "";
     }
	}

	if(buff != "")
    v.push_back(buff);

	return v;
}

void Gui::drawTextAligned(u32 font, s16 x, s16 y, color_t clr, const char* text, TextAlignment alignment) {
  u32 textWidth, textHeight;
  std::vector<std::string> lines;

  std::hash<std::string> hashFn;

  switch (alignment) {
    case ALIGNED_LEFT:
      lines = Gui::split(text, '\n');

      for (auto line : lines) {
        size_t strHash = hashFn(std::string(line));

        if (m_stringDimensions.find(strHash) == m_stringDimensions.end()) {
          getTextDimensions(font, line.c_str(), &textWidth, &textHeight);
          m_stringDimensions.insert(std::make_pair(strHash, std::pair<u16, u16>(textWidth, textHeight)));
        }

        drawText_(font, x, y, clr, text, 0, nullptr);
        y += m_stringDimensions[strHash].second;

      }
      break;
    case ALIGNED_CENTER:
      lines = Gui::split(text, '\n');

      for (auto line : lines) {
        size_t strHash = hashFn(std::string(line));

        if (m_stringDimensions.find(strHash) == m_stringDimensions.end()) {
          getTextDimensions(font, line.c_str(), &textWidth, &textHeight);
          m_stringDimensions.insert(std::make_pair(strHash, std::pair<u16, u16>(textWidth, textHeight)));
        }

        drawText_(font, x - (m_stringDimensions[strHash].first / 2.0F), y, clr, line.c_str(), 0, nullptr);
        y += m_stringDimensions[strHash].second;
      }
      break;
    case ALIGNED_RIGHT:
      lines = Gui::split(text, '\n');

      for (auto line : lines) {
        size_t strHash = hashFn(std::string(line));

        if (m_stringDimensions.find(strHash) == m_stringDimensions.end()) {
          getTextDimensions(font, line.c_str(), &textWidth, &textHeight);
          m_stringDimensions.insert(std::make_pair(strHash, std::pair<u16, u16>(textWidth, textHeight)));
        }

        drawText_(font, x - m_stringDimensions[strHash].first, y, clr, line.c_str(), 0, nullptr);
        y += m_stringDimensions[strHash].second;
      }
      break;

  }
}

void Gui::drawTextTruncate(u32 font, s16 x, s16 y, color_t clr, const char* text, u32 max_width, const char* end_text) {
  drawText_(font, x, y, clr, text, max_width, end_text);
}

void Gui::getTextDimensions(u32 font, const char* text, u32* width_out, u32* height_out) {
  u32 x = 0;
  u32 width = 0, height = 0;

  if (m_fontFacesTotal == 0) return;
  if (!setFontType(font)) return;
  m_fontLastUsedFace = m_fontFaces[0];

  while (*text) {
    glyph_t glyph;
    u32 codepoint = decodeUTF8(&text);

    //if (codepoint >= 0x0001 && codepoint <=)

    if (codepoint == '\n') {
      x = 0;
      height += m_fontLastUsedFace->size->metrics.height >> 6;
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
  height += m_fontLastUsedFace->size->metrics.height >> 6;

  if (width_out != nullptr)
    *width_out = width;

  if (height_out != nullptr)
    *height_out = height;
}

void Gui::drawImage(s16 x, s16 y, s16 width, s16 height, const u8 *image, ImageMode mode) {
  s32 tmpx, tmpy;
  s32 pos;
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
        case IMAGE_MODE_BGR24:
          pos = ((tmpy*width) + tmpx) * 3;
          current_color = makeColor(image[pos+2], image[pos+1], image[pos+0], 255);
          break;
        case IMAGE_MODE_ABGR32:
          pos = ((tmpy*width) + tmpx) * 4;
          current_color = makeColor(image[pos+3], image[pos+2], image[pos+1], image[pos+0]);
          break;
      }

      drawPixel(x+tmpx, y+tmpy, current_color);
    }
  }
}

void Gui::drawImage(s16 x, s16 y, s16 startx, s16 starty, s16 width, s16 height, const u8 *image, ImageMode mode) {
  s32 tmpx, tmpy;
  s32 pos;
  color_t current_color;

  for (tmpy = startx; tmpy < height; tmpy++) {
    for (tmpx = starty; tmpx < width; tmpx++) {
      switch (mode) {
        case IMAGE_MODE_RGB24:
          pos = ((tmpy*width) + tmpx) * 3;
          current_color = makeColor(image[pos+0], image[pos+1], image[pos+2], 255);
          break;
        case IMAGE_MODE_RGBA32:
          pos = ((tmpy*width) + tmpx) * 4;
          current_color = makeColor(image[pos+0], image[pos+1], image[pos+2], image[pos+3]);
          break;
        case IMAGE_MODE_BGR24:
          pos = ((tmpy*width) + tmpx) * 3;
          current_color = makeColor(image[pos+2], image[pos+1], image[pos+0], 255);
          break;
        case IMAGE_MODE_ABGR32:
          pos = ((tmpy*width) + tmpx) * 4;
          current_color = makeColor(image[pos+3], image[pos+2], image[pos+1], image[pos+0]);
          break;
      }

      drawPixel(x+tmpx, y+tmpy, current_color);
    }
  }
}

void Gui::drawRectangled(s16 x, s16 y, s16 w, s16 h, color_t color) {
  for (s32 j = y; j < y + h; j++) {
    for (s32 i = x; i < x + w; i++) {
      drawPixel(i, j, color);
    }
  }
}

void Gui::drawRectangle(s16 x, s16 y, s16 w, s16 h, color_t color) {
  for (s32 j = y; j < y + h; j++) {
    for (s32 i = x; i < x + w; i+=4) {
      draw4PixelsRaw(i, j, color);
    }
  }
}

void Gui::drawShadow(s16 x, s16 y, s16 width, s16 height) {
  color_t shadowColor;
  u8 shadowAlphaBase = 80;
  u8 shadowSize = 4;
  u8 shadowInset;

  y += height;

  for (s32 tmpx = x; tmpx < (x + width); tmpx+=4) {
    for (s32 tmpy = y; tmpy < (y + height); tmpy++) {
      shadowColor = makeColor(0, 0, 0, shadowAlphaBase * (1.0F - static_cast<float>(tmpy - y) / (static_cast<float>(shadowSize))));
      shadowInset = (tmpy - y);

      if (tmpx >= (x + shadowInset) && tmpx < (x + width - shadowInset))
        for (s32 i = 0; i < 4; i++) {
          if (tmpx < 0 || tmpy < 0) continue;
          drawPixel(tmpx + i,tmpy, shadowColor);
        }
    }
  }
}

void Gui::drawTooltip(s16 x, s16 y, const char *text, color_t backgroundColor, color_t textColor, u8 alpha, bool flipped) {
  backgroundColor.a = alpha;
  textColor.a = alpha;

  s16 tipX, tipY;
  for (tipY = 0; tipY < 20; tipY++) {
    for (tipX = x - tipY; tipX < x + tipY; tipX++) {
      if(tipX >= 0 && tipX < 1280 && tipY >= 0 && tipY <= 1280)
        Gui::drawPixel(tipX, (flipped ? -tipY : tipY) + y, backgroundColor);
      else continue;
    }
  }

  tipY += y;

  u32 textWidth, textHeight;
  Gui::getTextDimensions(font14, text, &textWidth, &textHeight);

  s16 overlapRight = std::max(static_cast<s32>((tipX + textWidth - 20)) - 1280, 0);
  s16 overlapLeft = std::min(tipX - 50 - overlapRight, 0);

  Gui::drawRectangled(tipX - 50 - overlapRight - overlapLeft, flipped ? tipY - textHeight - 69 : tipY, textWidth + 30, textHeight + 30, backgroundColor);
  Gui::drawTextAligned(font14, tipX - 35 + static_cast<s32>(textWidth) / 2 - overlapRight - overlapLeft, flipped ? tipY - textHeight - 59 : tipY + 10, textColor, text, ALIGNED_CENTER);
}

inline u8 getPixel(u8* in, size_t src_width, size_t src_height, u16 x, u16 y, s32 channel) {
  if (x < src_width && y < src_height)
    return in[(x * 3 * src_width) + (3 * y) + channel];

  return 0;
}

void Gui::resizeImage(u8* in, u8* out, size_t src_width, size_t src_height, size_t dest_width, size_t dest_height) {
  const float tx = static_cast<float>(src_width) / dest_width;
  const float ty = static_cast<float>(src_height) / dest_height;
  const s32 channels = 3;
  const size_t row_stride = dest_width * channels;

  u8 C[5] = { 0 };

  for (u32 i = 0; i < dest_height; ++i) {
    for (u32 j = 0; j < dest_width; ++j) {
      const s32 x = static_cast<u32>(tx * j);
      const s32 y = static_cast<u32>(ty * i);
      const float dx = tx * j - x;
      const float dy = ty * i - y;

      for (s32 k = 0; k < 3; ++k) {
        for (s32 jj = 0; jj < 4; ++jj) {
          const s32 z = y - 1 + jj;

          u8 a0 = getPixel(in, src_width, src_height, z, x, k);
          u8 d0 = getPixel(in, src_width, src_height, z, x - 1, k) - a0;
          u8 d2 = getPixel(in, src_width, src_height, z, x + 1, k) - a0;
          u8 d3 = getPixel(in, src_width, src_height, z, x + 2, k) - a0;
          u8 a1 = -1.0 / 3 * d0 + d2 - 1.0 / 6 * d3;
          u8 a2 = 1.0 / 2 * d0 + 1.0 / 2 * d2;
          u8 a3 = -1.0 / 6 * d0 - 1.0 / 2 * d2 + 1.0 / 6 * d3;
          C[jj] = a0 + a1 * dx + a2 * dx * dx + a3 * dx * dx * dx;

          d0 = C[0] - C[1];
          d2 = C[2] - C[1];
          d3 = C[3] - C[1];
          a0 = C[1];
          a1 = -1.0 / 3 * d0 + d2 -1.0 / 6 * d3;
          a2 = 1.0 / 2 * d0 + 1.0 / 2 * d2;
          a3 = -1.0 / 6 * d0 - 1.0 / 2 * d2 + 1.0 / 6 * d3;
          out[i * row_stride + j * channels + k] = a0 + a1 * dy + a2 * dy * dy + a3 * dy * dy * dy;
        }
      }
    }
  }
}

bool Gui::requestKeyboardInput(std::string headerText, std::string subHeaderText, std::string initialText, SwkbdType type, char *out_text, size_t maxLength) {
  SwkbdConfig kbd;
  swkbdCreate(&kbd, 0);
  swkbdConfigMakePresetDefault(&kbd);

  swkbdConfigSetInitialText(&kbd, initialText.c_str());
  swkbdConfigSetHeaderText(&kbd, headerText.c_str());
  swkbdConfigSetSubText(&kbd, subHeaderText.c_str());

  kbd.arg.arg.stringLenMax = maxLength;
  kbd.arg.arg.stringLenMaxExt = 1;
  kbd.arg.arg.textDrawType = SwkbdTextDrawType_Line;
  kbd.arg.arg.returnButtonFlag = false;
  kbd.arg.arg.type = type;
  kbd.arg.arg.keySetDisableBitmask = SwkbdKeyDisableBitmask_At | SwkbdKeyDisableBitmask_Percent | SwkbdKeyDisableBitmask_ForwardSlash | SwkbdKeyDisableBitmask_Backslash;

  swkbdShow(&kbd, out_text, maxLength + 1);
  swkbdClose(&kbd);

  return std::strcmp(out_text, "") != 0;
}

u128 Gui::requestPlayerSelection() {
  struct UserReturnData{
      u64 result;
      u128 UID;
  } PACKED;

  struct UserReturnData outdata;

  AppletHolder aph;
  AppletStorage ast;
  AppletStorage hast1;
  LibAppletArgs args;

  u8 indata[0xA0] = { 0 };
  indata[0x96] = 1;

  appletCreateLibraryApplet(&aph, AppletId_playerSelect, LibAppletMode_AllForeground);
  libappletArgsCreate(&args, 0);
  libappletArgsPush(&args, &aph);

  appletCreateStorage(&hast1, 0xA0);

  appletStorageWrite(&hast1, 0, indata, 0xA0);
  appletHolderPushInData(&aph, &hast1);
  appletHolderStart(&aph);

  while (appletHolderWaitInteractiveOut(&aph));

  appletHolderJoin(&aph);
  appletHolderPopOutData(&aph, &ast);
  appletStorageRead(&ast, 0, &outdata, 24);

  appletHolderClose(&aph);
  appletStorageClose(&ast);
  appletStorageClose(&hast1);

  return outdata.UID;
}

void Gui::requestErrorMessage(Result result) {
  AppletHolder aph;
  AppletStorage ast;
  AppletStorage hast1;
  LibAppletArgs args;
  u8 indata[20] = { 0 };

  *(Result*)&indata[16] = result;

  appletCreateLibraryApplet(&aph, AppletId_error, LibAppletMode_AllForeground);
  libappletArgsCreate(&args, 0);
  libappletArgsPush(&args, &aph);

  appletCreateStorage(&hast1, 20);

  appletStorageWrite(&hast1, 0, indata, 20);
  appletHolderPushInData(&aph, &hast1);

  appletHolderStart(&aph);

  while (appletHolderWaitInteractiveOut(&aph));

  appletHolderJoin(&aph);
  appletHolderPopOutData(&aph, &ast);

  appletHolderClose(&aph);
  appletStorageClose(&ast);
  appletStorageClose(&hast1);
}

void Gui::beginDraw() {
  this->framebuffer = (u8*)framebufferBegin(&Gui::g_fb_obj, &stride);
}

void Gui::endDraw() {
  if (Gui::g_currListSelector != nullptr)
    Gui::g_currListSelector->draw(this);

  if (Gui::g_currSnackbar != nullptr)
    Gui::g_currSnackbar->draw(this);

  if (Gui::g_currMessageBox != nullptr)
    Gui::g_currMessageBox->draw(this);

  framebufferEnd(&Gui::g_fb_obj);
}
