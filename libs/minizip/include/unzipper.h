#pragma once

#include <vector>
#include <istream>
#include <ostream>
#include <sstream>
#include <string>
#include <memory>
#include <map>

namespace zipper {
  
  class ZipEntry;

  class Unzipper
  {
  public:
    Unzipper(std::istream& buffer);
    Unzipper(std::vector<unsigned char>& buffer);
    Unzipper(const std::string& zipname);
    Unzipper(const std::string& zipname, const std::string& password);

    ~Unzipper(void);

    std::vector<ZipEntry> entries();

    bool extract(const std::string& destination, const std::map<std::string, std::string>& alternativeNames);
    bool extract(const std::string& destination=std::string());
    bool extractEntry(const std::string& name, const std::string& destination = std::string());
    bool extractEntryToStream(const std::string& name, std::ostream& stream);
    bool extractEntryToMemory(const std::string& name, std::vector<unsigned char>& vec);

    void close();

  private:
    std::string m_password;
    std::string m_zipname;
    std::istream& m_ibuffer;
    std::vector<unsigned char>& m_vecbuffer;
    bool m_usingMemoryVector;
    bool m_usingStream;
    bool m_open;

    struct Impl;
    Impl* m_impl;
  };


  class ZipEntry
  {
  private:
    typedef struct
    {
      unsigned int tm_sec;
      unsigned int tm_min;
      unsigned int tm_hour;
      unsigned int tm_mday;
      unsigned int tm_mon;
      unsigned int tm_year;
    } tm_s;

  public:
    ZipEntry(const std::string& name, unsigned long long int compressed_size, unsigned long long int uncompressed_size,
      int year, int month, int day, int hour, int minute, int second, unsigned long dosdate)
      : name(name), compressedSize(compressed_size), uncompressedSize(uncompressed_size), dosdate(dosdate)
    {
      // timestamp YYYY-MM-DD HH:MM:SS
      std::stringstream str;
      str << year << "-" << month << "-" << day <<
        " " << hour << ":" << minute << ":" << second;
      timestamp = str.str();

      unixdate.tm_year = year;
      unixdate.tm_mon = month;
      unixdate.tm_mday = day;
      unixdate.tm_hour = hour;
      unixdate.tm_min = minute;
      unixdate.tm_sec = second;
    }

    bool valid() { return !name.empty(); }

    std::string name, timestamp;
    unsigned long long int compressedSize, uncompressedSize;
    unsigned long dosdate;
    tm_s unixdate;
  };
}
