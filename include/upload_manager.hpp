#pragma once

#include <edizon.h>
#include <string>
#include <vector>

#include <curl/curl.h>

#include "helpers/title.hpp"

class UploadManager {
public:
  UploadManager();
  ~UploadManager();

  std::string upload(std::string path, std::string fileName, Title *title, std::string hash);

private:
  bool zip(std::vector<std::string> paths, std::vector<u8> *zipData);

  CURL *m_curl;
  std::string m_returnCode;

};