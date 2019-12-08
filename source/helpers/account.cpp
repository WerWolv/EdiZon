#include "helpers/account.hpp"

#include "guis/gui.hpp"

#include <cstring>
#include <memory>

extern "C" {
  #include "nanojpeg.h"
}

Account::Account(AccountUid userID) : m_userID(userID) {
  if (R_FAILED(accountGetProfile(&m_profile, userID))) return;

  if (!serviceIsActive(&m_profile.s)) return;

  if (R_FAILED(accountProfileGet(&m_profile, &m_userData, &m_profileBase))) return;
  if (R_FAILED(accountProfileGetImageSize(&m_profile, &m_profileImageSize))) return;

  m_userName = std::string(m_profileBase.nickname);

  std::vector<u8> buffer(m_profileImageSize);
  u8 *decodedBuffer;
  u32 imageSize = 0;

  if (R_FAILED(accountProfileLoadImage(&m_profile, &buffer[0], m_profileImageSize, &imageSize))) return;

  njInit();
  njDecode(&buffer[0], imageSize);

  m_profileImage.reserve(128 * 128 * 3);
  decodedBuffer = njGetImage();
  Gui::resizeImage(decodedBuffer, &m_profileImage[0], 256, 256, 128, 128);

  njDone();

  accountProfileClose(&m_profile);

  m_isInitialized = true;
}

Account::~Account() {
}

AccountUid Account::getUserID() {
  return m_userID;
}

std::string Account::getUserName() {
  return m_userName;
}

u8* Account::getProfileImage() {
  return &m_profileImage[0];
}

bool Account::isInitialized() {
  return m_isInitialized;
}