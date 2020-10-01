#include "helpers/title.hpp"

extern "C" {
  #include "nanojpeg.h"
}

Title::Title(FsSaveDataInfo& saveInfo) {
  Result rc=0;

  std::unique_ptr<NsApplicationControlData> buf = std::make_unique<NsApplicationControlData>();
  size_t outsize=0;

  NacpLanguageEntry *langentry = nullptr;

  if (buf == nullptr) {
    m_errorCode = 1;
    return;
  }
  memset(buf.get(), 0, sizeof(NsApplicationControlData));

  rc = nsGetApplicationControlData(NsApplicationControlSource_Storage, saveInfo.application_id & 0xFFFFFFFFFFFFFFF0, buf.get(), sizeof(NsApplicationControlData), &outsize);
  if (R_FAILED(rc)) {
    m_errorCode = 2;
    return;
  }

  if (outsize < sizeof(buf->nacp)) {
    m_errorCode = 3;
    return;
  }

  rc = nacpGetLanguageEntry(&buf->nacp, &langentry);
  if (R_FAILED(rc) || langentry==nullptr) {
    m_errorCode = 4;
    return;
  }

  m_titleName = std::string(langentry->name);
  m_titleAuthor = std::string(langentry->author);
  m_titleVersion = std::string(buf->nacp.display_version);

  m_titleID = saveInfo.application_id;

  njInit();

  size_t iconbytesize = outsize-sizeof(buf->nacp);
  size_t imagesize = 256 * 256 * 3;

  if (njDecode(buf->icon, iconbytesize) != NJ_OK) {
    m_errorCode = 5;
    njDone();
    return;
  }

  if (njGetWidth() != 256 || njGetHeight() != 256 || (size_t)njGetImageSize() != imagesize || njIsColor() != 1) {
    m_errorCode = 6;
    njDone();
    return;
  }

  u8* ptr = nullptr;

  ptr = njGetImage();
  if (ptr == nullptr) {
    m_errorCode = 7;
    njDone();
    return;
  }

  memcpy(m_titleIcon, ptr, imagesize);
  ptr = nullptr;

  njDone();
}

Title::~Title() {
}

std::string Title::getTitleName() {
  return m_titleName;
}

std::string Title::getTitleAuthor() {
  return m_titleAuthor;
}

std::string Title::getTitleVersion() {
  return m_titleVersion;
}

u8* Title::getTitleIcon() {
  return m_titleIcon;
}

u64 Title::getTitleID() {
  return m_titleID;
}

std::vector<AccountUid> Title::getUserIDs() {
  return m_userIDs;
}

void Title::addUserID(AccountUid userID) {
  m_userIDs.push_back(userID);
}
