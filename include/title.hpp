#pragma once

#include <switch.h>
#include <string>
#include "types.h"
#include "save.hpp"

class Title {
public:
  Title(FsSaveDataInfo saveDataInfo);
  ~Title();

  std::string getTitleName();
  uint8_t* getTitleIcon();
  std::vector<uint128_t> getUserIDs();
  void addUserID(uint128_t userID);
  uint64_t getTitleID();

private:
  uint8_t *m_titleIcon;
  char *m_titleName;
  uint64_t m_titleID;
  std::vector<uint128_t> m_userIDs;
};
