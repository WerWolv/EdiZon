#include "guis/gui_credits.hpp"

#include <thread>
#include <curl/curl.h>


#include "helpers/util.h"

static std::string m_remoteVersion, m_remoteCommitSha, m_remoteCommitMessage, m_localCommitSha;
static Thread networkThread;
static bool threadRunning;


static void getVersionInfoAsync(void* args);

GuiCredits::GuiCredits() : Gui() {
  if (!threadRunning) {
    threadRunning = true;
    threadCreate(&networkThread, getVersionInfoAsync, nullptr, 0x2000, 0x2C, -2);
    threadStart(&networkThread);
  }
}

GuiCredits::~GuiCredits() {

}

void GuiCredits::update() {
  Gui::update();
}

void GuiCredits::draw() {
  Gui::beginDraw();

  Gui::drawRectangle(0, 0, Gui::g_framebuffer_width, Gui::g_framebuffer_height, currTheme.backgroundColor);
  Gui::drawRectangle((u32)((Gui::g_framebuffer_width - 1220) / 2), 87, 1220, 1, currTheme.textColor);
  Gui::drawRectangle((u32)((Gui::g_framebuffer_width - 1220) / 2), Gui::g_framebuffer_height - 73, 1220, 1, currTheme.textColor);
  Gui::drawTextAligned(fontTitle, 70, 60, currTheme.textColor, "\uE017", ALIGNED_LEFT);
  Gui::drawTextAligned(font24, 70, 23, currTheme.textColor, "        Credits", ALIGNED_LEFT);

  Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 50, Gui::g_framebuffer_height - 51, currTheme.textColor, "\uE0E1 Back     \uE0E0 OK", ALIGNED_RIGHT);
  
  Gui::drawTextAligned(fontHuge, 100, 180, Gui::makeColor(0xFB, 0xA6, 0x15, 0xFF), "EdiZon v" VERSION_STRING, ALIGNED_LEFT);
  Gui::drawTextAligned(font20, 130, 190, currTheme.separatorColor, "by WerWolv", ALIGNED_LEFT);

  Gui::drawTextAligned(font14, 120, 250, currTheme.textColor, "Special thank to anybody who got involved into this project.", ALIGNED_LEFT);
  Gui::drawTextAligned(font14, 120, 270, currTheme.textColor, "Especially to all the config/cheat developers that brought this project to life!", ALIGNED_LEFT);

  Gui::drawTextAligned(font14, 900, 150, Gui::makeColor(0x51, 0x97, 0xF0, 0xFF), "Twitter: https://twitter.com/WerWolv", ALIGNED_LEFT);
  Gui::drawTextAligned(font14, 900, 175, Gui::makeColor(0x1A, 0x5E, 0xA7, 0xFF), "PayPal:  https://werwolv.net/donate", ALIGNED_LEFT);


  Gui::drawRectangled(50, 350, Gui::g_framebuffer_width - 100, 250, currTheme.textColor);
  Gui::drawRectangled(51, 351, Gui::g_framebuffer_width - 102, 248, currTheme.backgroundColor);
  Gui::drawShadow(52, 352, Gui::g_framebuffer_width - 104, 248);

  Gui::drawTextAligned(font20, 60, 360, currTheme.selectedColor, "EdiZon Update", ALIGNED_LEFT);

  Gui::drawTextAligned(font14, 80, 400, currTheme.textColor, std::string("Latest EdiZon version: " + (m_remoteVersion == "" ? "..." : m_remoteVersion)).c_str(), ALIGNED_LEFT);
  Gui::drawTextAligned(font14, 80, 425, currTheme.textColor, std::string("Latest database commit: [" + (m_remoteCommitSha == "" ? "..." : m_remoteCommitSha) + "] ").c_str(), ALIGNED_LEFT);
  Gui::drawTextAligned(font14, 90, 450, currTheme.separatorColor, (m_remoteCommitMessage == "" ? "..." : m_remoteCommitMessage.c_str()), ALIGNED_LEFT);

  Gui::endDraw();
}

void GuiCredits::onInput(u32 kdown) {
  if (kdown & KEY_B)
    Gui::g_nextGui = GUI_MAIN;
}

void GuiCredits::onTouch(touchPosition &touch) {

}

void GuiCredits::onGesture(touchPosition startPosition, touchPosition endPosition, bool finish) {

}

static size_t writeToStr(const char * contents, size_t size, size_t nmemb, std::string * userp){
    auto totalBytes = (size * nmemb);
    userp->append(contents, totalBytes);
    return totalBytes;
}

static void getVersionInfoAsync(void* args) {
  CURL *curl = curl_easy_init();

  struct curl_slist * headers = NULL;
  headers = curl_slist_append(headers, "Cache-Control: no-cache");

  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_URL, "http://vps.werwolv.net/api/edizon/v2/info/latest_edizon.php");
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToStr);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &m_remoteVersion);

  if (curl_easy_perform(curl) != CURLE_OK)
    m_remoteVersion = "???";

  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_URL, "http://vps.werwolv.net/api/edizon/v2/info/latest_db_sha.php");
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToStr);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &m_remoteCommitSha);

  if (curl_easy_perform(curl) != CURLE_OK)
    m_remoteCommitSha = "???";

  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_URL, "http://vps.werwolv.net/api/edizon/v2/info/latest_db_message.php");
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToStr);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &m_remoteCommitMessage);

  if (curl_easy_perform(curl) != CURLE_OK)
    m_remoteCommitMessage = "???";

  curl_easy_cleanup(curl);
}
