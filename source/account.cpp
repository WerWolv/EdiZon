#include "account.hpp"

#include "gui.hpp"

#include <cstring>

extern "C" {
  #include "nanojpeg.h"
}

Account *Account::g_currAccount = nullptr;
std::unordered_map<u128, Account*> Account::g_accounts;

Account::Account(u128 userID) : m_userID(userID) {
  accountInitialize();

  accountGetProfile(&m_profile, userID);
  accountProfileGet(&m_profile, &m_userData, &m_profileBase);
  accountProfileGetImageSize(&m_profile, &m_profileImageSize);

  m_userName = std::string(m_profileBase.username, 0x20);

  u8 *buffer = (u8*) malloc(m_profileImageSize);
  u8 *decodedBuffer = nullptr;
  size_t imageSize = 0;

  accountProfileLoadImage(&m_profile, buffer, m_profileImageSize, &imageSize);
  njInit();
  njDecode(buffer, imageSize);

  decodedBuffer = njGetImage();
  m_profileImage = (u8*) malloc(128*128*3);
  Gui::resizeImage(decodedBuffer, m_profileImage, 256, 256, 128, 128);

  njDone();

  accountProfileClose(&m_profile);
  accountExit();
}

Account::~Account() {

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
