#include "upload_manager.hpp"

#include "zipper.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <iterator>
#include <dirent.h>
#include <vector>
#include <string>
#include <cstdio>
#include <filesystem>

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "gui.hpp"

UploadManager::UploadManager() {
  curl_global_init(CURL_GLOBAL_DEFAULT);
  m_curl = curl_easy_init();

  if (!m_curl)
    printf("Curl initialization failed!\n");
}

UploadManager::~UploadManager() {
  curl_easy_cleanup(m_curl);
  curl_global_cleanup();
}

size_t writefunc(void *ptr, size_t size, size_t nmemb, std::string* s)
{
  *s += reinterpret_cast<char*>(ptr);
  return size*nmemb;
}

std::vector<std::string> listFiles(const std::string &path) {
  std::vector<std::string> paths;
    if (auto dir = opendir(path.c_str())) {
        while (auto f = readdir(dir)) {
            if (!f->d_name || f->d_name[0] == '.') continue;
            if (f->d_type == DT_DIR) 
                listFiles(path + f->d_name + "/");

            if (f->d_type == DT_REG)
              paths.push_back(path + "/" + f->d_name);
        }
        closedir(dir);
    }

    return paths;
}
 
static int xferinfo(void *p, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
  if (Gui::g_currMessageBox != nullptr)
    Gui::g_currMessageBox->setProgress(50.0F + ((ulnow / static_cast<float>(ultotal)) * 100) / 2.0F);

  hidScanInput();

  return hidKeysDown(CONTROLLER_P1_AUTO) & KEY_B;
}

std::string UploadManager::upload(std::string path, std::string fileName) {
  if (gethostid() == INADDR_LOOPBACK) return "";

  printf("%s\n", path.c_str());

  std::vector<std::string> filePaths;
  std::filesystem::recursive_directory_iterator end;
  for (std::filesystem::recursive_directory_iterator it(path); it != end; ++it)
    if (!it->is_directory())
      filePaths.push_back(it->path().c_str());

  if (!this->zip(filePaths)) return "";

  FILE *tar = fopen("/EdiZon/tmp/archive.zip", "rb");
  if (tar == nullptr) return "";
  fseek(tar, 0L, SEEK_END);
  size_t size = ftell(tar);
  char *data = (char*)malloc(size);

  rewind(tar);

  if (data == nullptr) return "";

  fread(data, 1, size, tar);
  fclose(tar);

  if (data == nullptr) {
    free(data);
    return "";
  }

  curl_mime *mime;
  curl_mimepart *part;

  mime = curl_mime_init(m_curl);
  part = curl_mime_addpart(mime);

  curl_mime_data(part, data, size);
  curl_mime_filename(part, std::string(fileName + ".zip").c_str());
  curl_mime_name(part, "file");

  curl_easy_setopt(m_curl, CURLOPT_MIMEPOST, mime);
  curl_easy_setopt(m_curl, CURLOPT_URL, "http://werwolv.teamatlasnx.com/v1/upload");
  curl_easy_setopt(m_curl, CURLOPT_CUSTOMREQUEST, "POST");
  curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, writefunc);
  curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &m_returnAddress);
  curl_easy_setopt(m_curl, CURLOPT_XFERINFOFUNCTION, &xferinfo);
  curl_easy_setopt(m_curl, CURLOPT_NOPROGRESS, 0L);

  if (curl_easy_perform(m_curl) != CURLE_OK) m_returnAddress = "";

  /* Clean-up. */
  curl_mime_free(mime);

  free(data);

  remove("/EdiZon/tmp/archive.zip");

  return m_returnAddress;
}

bool UploadManager::zip(std::vector<std::string> paths) {
  zipper::Zipper archive("/EdiZon/tmp/archive.zip");


  u8 pathOffset = 25;
  u32 currFile = 0;

  if (paths[0].find("/EdiZon/batch/") != std::string::npos)
    pathOffset = 14;
  else if (paths[0].find("/EdiZon/restore/") != std::string::npos)
    pathOffset = 16;

  for(auto path : paths) { 
    std::ifstream file = std::ifstream(path);

    printf("%s\n", path.c_str());

    archive.add(file, &path.c_str()[pathOffset], zipper::Zipper::Better);

    if (Gui::g_currMessageBox != nullptr)
      Gui::g_currMessageBox->setProgress(((++currFile / static_cast<float>(paths.size())) * 100) / 2.0F);

    hidScanInput();
    if (hidKeysDown(CONTROLLER_P1_AUTO) & KEY_B)
      return false;

    file.close();
  }

  archive.close();

  return true;
}