#pragma once

#include <edizon.h>

#include <string>
#include <vector>
#include <unordered_map>

class Account {
public:
  static inline Account *g_currAccount = nullptr;
  static inline std::unordered_map<u128, Account*> g_accounts;

  static inline u128 g_activeUser = 0;

  Account(u128 userID);
  ~Account();

  std::string getUserName();
  u128 getUserID();
  u8* getProfileImage();
  bool isInitialized();

private:
  AccountProfile m_profile;
  AccountUserData m_userData;
  AccountProfileBase m_profileBase;

  u128 m_userID;
  std::vector<u8> m_profileImage;
  size_t m_profileImageSize;
  std::string m_userName;
  bool m_isInitialized;
};
