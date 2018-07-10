#include "update_manager.hpp"

#include <stdio.h>

#include <iostream>
#include <cstring>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <curl/curl.h>

#define EDIZON_URL "http://werwolv.net/EdiZon"

CURL *curl;

UpdateManager::UpdateManager(u64 titleID) {
  curl_global_init(CURL_GLOBAL_DEFAULT);
  curl = curl_easy_init();

  if (!curl)
    printf("Curl initialization failed!\n");
}

UpdateManager::~UpdateManager() {
  curl_easy_cleanup(curl);
  curl_global_cleanup();
}

const std::vector<std::string> split(const std::string& s, const char& c) {
	std::string buff {""};
	std::vector<std::string> v;

	for(auto n:s) {
		if (n != c) buff += n;
    else if (n == c && buff != "") {
       v.push_back(buff);
        buff = "";
     }
	}

	if(buff != "")
    v.push_back(buff);

	return v;
}

size_t writeToString(void *contents, size_t size, size_t nmemb, void *userp){
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

size_t writeToFile(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

std::vector<std::pair<std::string, std::string>> parseUpdateFile(std::string content, std::string *out_versionString) {
  std::vector<std::pair<std::string, std::string>> updatePathPair;
  std::vector<std::string> lines = split(content, '\n');

  *out_versionString = lines[0];
  lines.erase(lines.begin());

  for (auto line : lines) {
    std::vector<std::string> paths = split(line, ' ');
    updatePathPair.push_back(std::make_pair(std::string(EDIZON_URL) + paths[0], paths[1]));
  }

  return updatePathPair;
}

void trim(std::string& s) {
    while(s.compare(0,1," ")==0)
        s.erase(s.begin());
    while(s.size()>0 && s.compare(s.size()-1,1," ")==0)
        s.erase(s.end()-1);
}

bool UpdateManager::checkUpdate() {
  CURLcode res;
  FILE *fp;
  std::string str;

  curl_easy_setopt(curl, CURLOPT_URL, EDIZON_URL"/dir.php");
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToString);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &str);

  res = curl_easy_perform(curl);

  if (res != CURLE_OK) {
    printf("CURL perform failed: %s\n", curl_easy_strerror(res));
    return false;
  }

  m_downloadPaths = parseUpdateFile(str, &m_versionString);

  mkdir("/EdiZon", 0777);
  mkdir("/EdiZon/editor", 0777);
  mkdir("/EdiZon/editor/scripts", 0777);
  mkdir("/EdiZon/editor/scripts/lib", 0777);

  fp = fopen("/EdiZon/update.txt", "r+");

  if (fp != nullptr) {
    u8 fileLength;
    char *localVersionString;

    fseek(fp, 0L, SEEK_END);
    fileLength = ftell(fp);
    rewind(fp);

    localVersionString = new char[fileLength];

    fread(localVersionString, sizeof(char), fileLength, fp);

    if (std::string(localVersionString) == m_versionString) {
      fclose(fp);
      return false;
    }
  }

  fclose(fp);

  fp = fopen("/EdiZon/update.txt", "w+");
  fputs(m_versionString.c_str(), fp);

  fclose(fp);

  for (auto path : m_downloadPaths) {
    fp = fopen(path.second.c_str(), "wb");

    curl_easy_setopt(curl, CURLOPT_URL, path.first.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToFile);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

    res = curl_easy_perform(curl);

    if (res != CURLE_OK)
      printf("CURL perform failed: %s\n", curl_easy_strerror(res));

    fclose(fp);
  }

  return true;

}
