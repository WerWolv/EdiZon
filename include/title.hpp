#pragma once

#include <switch.h>
#include <string>
#include "types.h"
#include "save.hpp"

class Title {
public:
  Title(FsSaveDataInfo& saveInfo);
  ~Title();

  std::string name();
  u8* icon();
  std::vector<u128> userIDs();
  void userID(u128 userID);
  int errorCode();

private:
  u8 *m_titleIcon;
  std::string m_titleName;
  size_t m_errorCode;
  std::vector<u128> m_userIDs;
};
