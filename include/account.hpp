#pragma once

#include <switch.h>

#include <string>

class Account {
public:
  Account(u128 userID);
  ~Account();

  std::string getUserName();
  u128 getUserID();
  u8* getProfileImage();

private:
  AccountProfile m_profile;
  AccountUserData m_userData;
  AccountProfileBase m_profileBase;

  u128 m_userID;
  u8 *m_profileImage;
  size_t m_profileImageSize;
  std::string m_userName;

  void resizeImage(u8* in, u8* out, size_t src_width, size_t src_height, size_t dest_width, size_t dest_height);
};
