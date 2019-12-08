#pragma once

#include <edizon.h>
#include <string>
#include <map>
#include <memory>

#include "types.h"
#include "helpers/save.hpp"
#include "helpers/account.hpp"

class Title {
public:
  static inline Title *g_currTitle;
  static inline std::map<u64, Title*> g_titles;

  static inline u64 g_activeTitle = 0;

  Title(FsSaveDataInfo& saveInfo);
  ~Title();

  std::string getTitleName();
  std::string getTitleAuthor();
  std::string getTitleVersion();
  u8* getTitleIcon();
  std::vector<AccountUid> getUserIDs();
  void addUserID(AccountUid userID);
  u64 getTitleID();

private:
  u8 m_titleIcon[256*256*3];
  u64 m_titleID;
  std::string m_titleName;
  std::string m_titleAuthor;
  std::string m_titleVersion;
  std::vector<AccountUid> m_userIDs;
  u8 m_errorCode;
};
