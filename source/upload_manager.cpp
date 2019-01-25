#include "upload_manager.hpp"

#include "tarball.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <iterator>
#include <dirent.h>
#include <vector>
#include <string>
#include <cstdio>

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

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

std::string UploadManager::upload(std::string path, std::string fileName) {
  if (gethostid() == INADDR_LOOPBACK) return "";
  if (!this->zip(listFiles(path))) return "";

  FILE *tar = fopen("/EdiZon/tmp/archive.tar", "rb");
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

  /* Build an HTTP form with a single field named "data", */
  mime = curl_mime_init(m_curl);
  part = curl_mime_addpart(mime);

  curl_mime_data(part, data, size);
  curl_mime_filename(part, std::string(fileName + ".tar").c_str());
  curl_mime_name(part, "file");

  /* Post and send it. */
  curl_easy_setopt(m_curl, CURLOPT_MIMEPOST, mime);
  curl_easy_setopt(m_curl, CURLOPT_URL, "http://werwolv.teamatlasnx.com/v1/upload");
  curl_easy_setopt(m_curl, CURLOPT_CUSTOMREQUEST, "POST");
  curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, writefunc);
  curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &m_returnAddress);

  if (curl_easy_perform(m_curl) != CURLE_OK) m_returnAddress = "";

  /* Clean-up. */
  curl_mime_free(mime);

  free(data);

  remove("/EdiZon/tmp/archive.tar");

  return m_returnAddress;
}

bool UploadManager::zip(std::vector<std::string> paths) {
  std::fstream buf("/EdiZon/tmp/archive.tar", std::ios::out);

  if (!buf.is_open()) return false;

  lindenb::io::Tar tarball(buf);

  for(auto path : paths)
    tarball.putFile(path.c_str(), &path.c_str()[1]);

  tarball.finish();

  buf.close();

  return true;
}