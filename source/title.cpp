#include "title.hpp"

extern "C" {
#include "nanojpeg.h"
}

Title *Title::g_currTitle = nullptr;

Title::Title(FsSaveDataInfo& saveInfo) {
  Result rc=0;

  NsApplicationControlData *buf=NULL;
  size_t outsize=0;

  NacpLanguageEntry *langentry = NULL;

  buf = (NsApplicationControlData*)malloc(sizeof(NsApplicationControlData));
  if (buf==NULL) {
    m_errorCode = 1;
    return;
  }
  memset(buf, 0, sizeof(NsApplicationControlData));

  rc = nsInitialize();
  if (R_FAILED(rc)) {
    m_errorCode = 2;
    return;
  }

  rc = nsGetApplicationControlData(1, saveInfo.titleID, buf, sizeof(NsApplicationControlData), &outsize);
  if (R_FAILED(rc)) {
    m_errorCode = 3;
    return;
  }

  if (outsize < sizeof(buf->nacp)) {
    m_errorCode = 4;
    return;
  }

  rc = nacpGetLanguageEntry(&buf->nacp, &langentry);
  if (R_FAILED(rc) || langentry==NULL) {
    m_errorCode = 5;
    return;
  }

  m_titleName = std::string(langentry->name);
  m_titleAuthor = std::string(langentry->author);

  m_titleID = buf->nacp.titleID0;

  njInit();

  size_t iconbytesize = outsize-sizeof(buf->nacp);
  size_t imagesize = 256*256*3;

  if (njDecode(buf->icon, iconbytesize) != NJ_OK) {
    m_errorCode = 6;
    njDone();
    return;
  }

  if (njGetWidth() != 256 || njGetHeight() != 256 || (size_t)njGetImageSize() != imagesize || njIsColor() != 1) {
    m_errorCode = 7;
    njDone();
    return;
  }

  u8* ptr = NULL;

  ptr = njGetImage();
  if (ptr == NULL) {
    m_errorCode = 8;
    njDone();
    return;
  }

  m_titleIcon = (u8*)malloc(imagesize);
  memcpy(m_titleIcon, ptr, imagesize);
  ptr = NULL;

  njDone();
  nsExit();
}

Title::~Title() {
  free(m_titleIcon);
}

std::string Title::getTitleName() {
  return m_titleName;
}

std::string Title::getTitleAuthor() {
  return m_titleAuthor;
}

u8* Title::getTitleIcon() {
  return m_titleIcon;
}

u64 Title::getTitleID() {
  return m_titleID;
}

std::vector<u128> Title::getUserIDs() {
  return m_userIDs;
}

void Title::addUserID(u128 userID) {
  m_userIDs.push_back(userID);
}
