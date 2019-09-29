#include "guis/gui_about.hpp"

#include <thread>
#include <curl/curl.h>

#include "update_manager.hpp"
#include "helpers/util.h"
#include "helpers/config.hpp"

static std::string remoteVersion, remoteCommitSha, remoteCommitMessage;
static Thread networkThread;
static bool threadRunning;
static bool updateAvailable;

static void getVersionInfoAsync(void* args);

GuiAbout::GuiAbout() : Gui() {
  updateAvailable = false;
  
  remoteVersion = "";
  remoteCommitSha = "";
  remoteCommitMessage = "";

  if (!threadRunning) {
    threadRunning = true;
    threadCreate(&networkThread, getVersionInfoAsync, nullptr, 0x2000, 0x2C, -2);
    threadStart(&networkThread);
  }
}

GuiAbout::~GuiAbout() {

}

void GuiAbout::update() {
  Gui::update();
}

void GuiAbout::draw() {
  Gui::beginDraw();

  Gui::drawRectangle(0, 0, Gui::g_framebuffer_width, Gui::g_framebuffer_height, currTheme.backgroundColor);
  Gui::drawRectangle((u32)((Gui::g_framebuffer_width - 1220) / 2), 87, 1220, 1, currTheme.textColor);
  Gui::drawRectangle((u32)((Gui::g_framebuffer_width - 1220) / 2), Gui::g_framebuffer_height - 73, 1220, 1, currTheme.textColor);
  Gui::drawTextAligned(fontTitle, 70, 60, currTheme.textColor, "\uE017", ALIGNED_LEFT);
  Gui::drawTextAligned(font24, 70, 23, currTheme.textColor, "        About", ALIGNED_LEFT);

  if (updateAvailable)
    Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 50, Gui::g_framebuffer_height - 51, currTheme.textColor, "\uE0F0 Install update     \uE0E1 Back     \uE0E0 OK", ALIGNED_RIGHT);
  else
    Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 50, Gui::g_framebuffer_height - 51, currTheme.textColor, "\uE0E1 Back     \uE0E0 OK", ALIGNED_RIGHT);

  Gui::drawTextAligned(fontHuge, 100, 180, Gui::makeColor(0xFB, 0xA6, 0x15, 0xFF), "EdiZon v" VERSION_STRING, ALIGNED_LEFT);
  Gui::drawTextAligned(font20, 130, 190, currTheme.separatorColor, "by WerWolv", ALIGNED_LEFT);

  Gui::drawTextAligned(font14, 120, 250, currTheme.textColor, "Special thank to anybody who got involved into this project.", ALIGNED_LEFT);
  Gui::drawTextAligned(font14, 120, 270, currTheme.textColor, "Especially to all the config/cheat developers that brought this project to life!", ALIGNED_LEFT);

  Gui::drawTextAligned(font14, 900, 250, Gui::makeColor(0x51, 0x97, 0xF0, 0xFF), "Twitter: https://twitter.com/WerWolv", ALIGNED_LEFT);
  Gui::drawTextAligned(font14, 900, 275, Gui::makeColor(0x1A, 0x5E, 0xA7, 0xFF), "PayPal:  https://werwolv.net/donate", ALIGNED_LEFT);


  Gui::drawRectangled(50, 350, Gui::g_framebuffer_width - 100, 250, currTheme.textColor);
  Gui::drawRectangled(51, 351, Gui::g_framebuffer_width - 102, updateAvailable ? 190 : 248, currTheme.backgroundColor);
  Gui::drawShadow(52, 352, Gui::g_framebuffer_width - 104, 248);

  if (updateAvailable)
    Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 2, 555, currTheme.backgroundColor, "A update for EdiZon, save editor or cheat is available!", ALIGNED_CENTER);

  Gui::drawTextAligned(font20, 60, 360, currTheme.selectedColor, "EdiZon Update", ALIGNED_LEFT);

  Gui::drawTextAligned(font14, 80, 400, currTheme.textColor, std::string("Latest EdiZon version: " + (remoteVersion == "" ? "..." : remoteVersion)).c_str(), ALIGNED_LEFT);
  Gui::drawTextAligned(font14, 80, 425, currTheme.textColor, std::string("Latest database commit: [ " + (remoteCommitSha == "" ? "..." : remoteCommitSha) + " ] ").c_str(), ALIGNED_LEFT);
  Gui::drawTextAligned(font14, 90, 450, currTheme.separatorColor, (remoteCommitMessage == "" ? "..." : remoteCommitMessage.c_str()), ALIGNED_LEFT);

  Gui::endDraw();
}

void GuiAbout::onInput(u32 kdown) {
  if (kdown & KEY_B) {
    if (threadRunning) {
      threadWaitForExit(&networkThread);
      threadClose(&networkThread);
      threadRunning = false;
    }

    Gui::g_nextGui = GUI_MAIN;
  }

  if (kdown & KEY_MINUS && updateAvailable) {
    UpdateManager updateManager;

    (new MessageBox("Updating cheats, configs and EdiZon.\n \nThis may take a while...", MessageBox::NONE))->show();
    requestDraw();

    Updates updates = updateManager.checkUpdate();

    switch (updates) {
      case NONE: (new MessageBox("Latest cheats and save editors are already installed!", MessageBox::OKAY))->show(); break;
      case ERROR: (new MessageBox("An error while downloading the updates has occured!", MessageBox::OKAY))->show(); break;
      case EDITOR: (new MessageBox("Updated save editors and cheats to the latest version!", MessageBox::OKAY))->show(); break;
      case EDIZON: (new MessageBox("Updated EdiZon, save editors and cheats and \n scripts tothe latest version! \n Please restart EdiZon!", MessageBox::OKAY))->show(); break;
    }

    if (updates == EDITOR || updates == EDIZON) {
      memcpy(Config::getConfig()->latestCommit, remoteCommitSha.c_str(), remoteCommitSha.size());
      Config::writeConfig();
    }
    
    updateAvailable = (strcmp(remoteCommitSha.c_str(), Config::getConfig()->latestCommit) != 0 && strcmp(remoteCommitSha.c_str(), "???") != 0);
  }
}

void GuiAbout::onTouch(touchPosition &touch) {

}

void GuiAbout::onGesture(touchPosition startPosition, touchPosition endPosition, bool finish) {

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
  curl_easy_setopt(curl, CURLOPT_URL, EDIZON_URL "/info/latest_edizon.php");
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToStr);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &remoteVersion);

  remoteVersion = "";
  if (curl_easy_perform(curl) != CURLE_OK)
    remoteVersion = "???";

  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_URL, EDIZON_URL "/info/latest_db_sha.php");
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToStr);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &remoteCommitSha);

  remoteCommitSha = "";

  if (curl_easy_perform(curl) != CURLE_OK)
    remoteCommitSha = "???";

  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_URL, EDIZON_URL "/info/latest_db_message.php");
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToStr);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &remoteCommitMessage);

  remoteCommitMessage = "";

  if (curl_easy_perform(curl) != CURLE_OK)
    remoteCommitMessage = "???";

  curl_easy_cleanup(curl);

  updateAvailable = (strcmp(remoteCommitSha.c_str(), Config::getConfig()->latestCommit) != 0 && strcmp(remoteCommitSha.c_str(), "???") != 0);
}
