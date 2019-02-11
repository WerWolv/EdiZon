#pragma once

#include <switch.h>
#include <string>
#include <vector>

#include <curl/curl.h>

class UploadManager {
public:
  UploadManager();
  ~UploadManager();

  std::string upload(std::string path, std::string fileName);

private:
  bool zip(std::vector<std::string> paths, std::vector<u8> *zipData);

  CURL *m_curl;
  std::string m_returnAddress;

};