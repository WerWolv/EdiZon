#include "title.hpp"

Title::Title(FsSaveDataInfo saveDataInfo) {
  //_getTitleName(saveDataInfo.titleID, this->m_titleName);
    this->m_titleID = saveDataInfo.titleID;
  _getTitleIcon(this->m_titleID, &this->m_titleIcon);
}

Title::~Title() {
  free(*this->m_titleIcon);
  free(this->m_titleIcon);
}

std::string Title::getTitleName() {
  return this->m_titleName;
}

uint8_t* Title::getTitleIcon() {
  return this->m_titleIcon;
}

std::vector<uint128_t> Title::getUserIDs() {
  return this->m_userIDs;
}

void Title::addUserID(u128 userID) {
  this->m_userIDs.push_back(userID);
}

uint64_t Title::getTitleID() {
  return this->m_titleID;
}
