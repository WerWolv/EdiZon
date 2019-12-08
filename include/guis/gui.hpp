#pragma once

#include <edizon.h>
#include <cstring>
#include <unordered_map>

#include "helpers/title.hpp"

#include "ui_elements/snackbar.hpp"
#include "ui_elements/list_selector.hpp"
#include "ui_elements/message_box.hpp"

#include "theme.h"
#include "types.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#define FONT_FACES_MAX PlSharedFontType_Total

#define fontHuge 5
#define fontTitle 4
#define font24 3
#define font20 2
#define font14 1

enum gui_t {
  GUI_INVALID,
  GUI_MAIN,
  GUI_EDITOR,
  GUI_TX_WARNING,
  GUI_CHEATS,
  GUI_GUIDE,
  GUI_ABOUT
};

class Gui {
public:
  static inline enum gui_t g_nextGui = GUI_INVALID;

  u8 *framebuffer;
  static inline Framebuffer g_fb_obj;
  static inline u32 g_framebuffer_width = 1280;
  static inline u32 g_framebuffer_height = 720;

  static inline bool g_splashDisplayed = false;
  static inline bool g_requestExit = false;

  static inline Snackbar *g_currSnackbar = nullptr;
  static inline ListSelector *g_currListSelector = nullptr;
  static inline MessageBox *g_currMessageBox = nullptr;

  Gui();
  virtual ~Gui();

  virtual void update();
  virtual void draw() = 0;
  virtual void onInput(u32 kdown) = 0;
  virtual void onTouch(touchPosition &touch) = 0;
  virtual void onGesture(touchPosition startPosition, touchPosition endPosition, bool finish) = 0;

  static void resizeImage(u8* in, u8* out, size_t src_width, size_t src_height, size_t dest_width, size_t dest_height);
  static std::vector<std::string> split(const std::string& s, const char& c);
  inline u8 blendColor(u32 src, u32 dst, u8 alpha);
  color_t makeColor(u8 r, u8 g, u8 b, u8 a);
  void drawRectangle(s16 x, s16 y, s16 w, s16 h, color_t color);
  void drawRectangled(s16 x, s16 y, s16 w, s16 h, color_t color);

  bool fontInit();
  void fontExit();

  void drawText(u32 font, s16 x, s16 y, color_t clr, const char* text);
  void drawTextAligned(u32 font, s16 x, s16 y, color_t clr, const char* text, TextAlignment alignment);
  void drawTextTruncate(u32 font, s16 x, s16 y, color_t clr, const char* text, u32 max_width, const char* end_text);
  void getTextDimensions(u32 font, const char* text, u32* width_out, u32* height_out);

  void drawImage(s16 x, s16 y, s16 width, s16 height, const u8 *image, ImageMode mode);
  void drawImage(s16 x, s16 y, s16 startx, s16 starty, s16 width, s16 height, const u8 *image, ImageMode mode);
  void drawShadow(s16 x, s16 y, s16 width, s16 height);
  void drawTooltip(s16 x, s16 y, const char *text, color_t backgroundColor, color_t textColor, u8 alpha, bool flipped);

  static bool requestKeyboardInput(std::string headerText, std::string subHeaderText, std::string initialText, SwkbdType type, char *out_text, size_t maxLength);
  static AccountUid requestPlayerSelection();
  static void requestErrorMessage(Result result);

protected:
  void beginDraw();
  void endDraw();

private:
  FT_Error m_fontLibret, m_fontFacesRet[FONT_FACES_MAX];
  FT_Library m_fontLibrary;
  FT_Face m_fontFaces[FONT_FACES_MAX];
  FT_Face m_fontLastUsedFace;
  s32 m_fontFacesTotal;

  std::unordered_map<size_t, std::pair<u16, u16>> m_stringDimensions;

  void drawText_(u32 font, s16 x, s16 y, color_t clr, const char* text, s32 max_width, const char* end_text);
  inline void draw4PixelsRaw(s16 x, s16 y, color_t clr);
  inline bool fontLoadGlyph(glyph_t* glyph, u32 font, u32 codepoint);
  void drawGlyph(s16 x, s16 y, color_t clr, const glyph_t* glyph);
  bool setFontType(u32 font);
  inline u8 decodeByte(const char** ptr);
  inline s8 decodeUTF8Cont(const char** ptr);
  inline u32 decodeUTF8(const char** ptr);
  inline void drawPixel(s16 x, s16 y, color_t clr);
};

#define COLOR_WHITE makeColor(255, 255, 255, 255)
#define COLOR_BLACK makeColor(0, 0, 0, 255)

extern void requestDraw();