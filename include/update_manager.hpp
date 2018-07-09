#pragma once

#include <switch.h>
#include <string>
#include <vector>

class UpdateManager {
public:
  UpdateManager(u64 titleID);
  ~UpdateManager();

  bool checkUpdate();
private:
  u64 m_titleID;
  std::vector<std::pair<std::string, std::string>> m_downloadPaths;
  std::string m_versionString;

};
