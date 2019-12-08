#pragma once

#include <edizon.h>

#include <string>
#include <vector>
#include <unordered_map>

namespace std {
  template <>
  struct hash<AccountUid> {
    std::size_t operator()(const AccountUid& acc_uid) const {
      using std::hash;
      return ((hash<u64>()(acc_uid.uid[0])
               ^ (hash<u64>()(acc_uid.uid[1]) << 1)) >> 1);
    }
  };
}

inline bool operator==(const AccountUid& acc_1, const AccountUid& acc_2) {
  return (acc_1.uid[0] == acc_2.uid[0] && acc_1.uid[1] == acc_2.uid[1]);
}

class Account {
public:
  static inline Account *g_currAccount = nullptr;
  static inline std::unordered_map<AccountUid, Account*> g_accounts;

  static inline AccountUid g_activeUser = {0};

  Account(AccountUid userID);
  ~Account();

  std::string getUserName();
  AccountUid getUserID();
  u8* getProfileImage();
  bool isInitialized();

private:
  AccountProfile m_profile;
  AccountUserData m_userData;
  AccountProfileBase m_profileBase;

  AccountUid m_userID;
  std::vector<u8> m_profileImage;
  u32 m_profileImageSize;
  std::string m_userName;
  bool m_isInitialized;
};
