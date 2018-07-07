#include "account.hpp"

#include "gui.hpp"

#include <cstring>

extern "C" {
  #include "nanojpeg.h"
}

Account::Account(u128 userID) : m_userID(userID) {
  accountInitialize();

  accountGetProfile(&m_profile, userID);
  accountProfileGet(&m_profile, &m_userData, &m_profileBase);
  accountProfileGetImageSize(&m_profile, &m_profileImageSize);

  m_userName = std::string(m_profileBase.username, 0x20);

  u8 *buffer = new u8[m_profileImageSize];
  u8 *decodedBuffer = nullptr;
  size_t imageSize = 0;

  accountProfileLoadImage(&m_profile, buffer, m_profileImageSize, &imageSize);
  njInit();
  njDecode(buffer, imageSize);

  decodedBuffer = njGetImage();
  m_profileImage = new u8[128 * 128 * 3];
  Gui::resizeImage(decodedBuffer, m_profileImage, 256, 256, 128, 128);

  njDone();

  delete[] buffer;

  accountProfileClose(&m_profile);
  accountExit();
}

Account::~Account() {
  delete[] m_profileImage;
}

u128 Account::getUserID() {
  return m_userID;
}

std::string Account::getUserName() {
  return m_userName;
}

u8* Account::getProfileImage() {
  return m_profileImage;
}
