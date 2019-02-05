#pragma once

#include <switch.h>
#include <string>
#include <vector>

#include <curl/curl.h>

class UploadManager {
public:
  UploadManager();
  ~UploadManager();

  std::string upload(std::string path, std::string fileName, u64 tid);

private:
  bool zip(std::vector<std::string> paths);

  CURL *m_curl;
  std::string m_returnAddress;

};