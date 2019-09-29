#include "tools.h"
#include "defs.h"
#include <algorithm>
#include <iterator>

#include <CDirEntry.h>

#include <cstdio>

#if defined(WIN32) && !defined(CYGWIN)
#  include "tps/dirent.h"
#  include "tps/dirent.c"
#else
#  include <sys/types.h>
#  include <dirent.h>
#  include <unistd.h>
#endif  /* WIN32 */



namespace zipper {

  /* calculate the CRC32 of a file,
  because to encrypt a file, we need known the CRC32 of the file before */

  void getFileCrc(std::istream& input_stream, std::vector<char>& buff, unsigned long& result_crc)
  {
    unsigned long calculate_crc = 0;
    unsigned long size_read = 0;
    unsigned long total_read = 0;

    do {
      input_stream.read(buff.data(), buff.size());
      size_read = (unsigned long)input_stream.gcount();

      if (size_read > 0)
        calculate_crc = crc32(calculate_crc, (const unsigned char*)buff.data(), size_read);

      total_read += size_read;

    } while (size_read > 0);

    input_stream.seekg(0);
    result_crc = calculate_crc;
  }

  bool isLargeFile(std::istream& input_stream)
  {
    ZPOS64_T pos = 0;
    input_stream.seekg(0, std::ios::end);
    pos = input_stream.tellg();
    input_stream.seekg(0);

    return pos >= 0xffffffff;
  }

  bool checkFileExists(const std::string& filename)
  {
    return CDirEntry::exist(filename);
  }

  bool makedir(const std::string& newdir)
  {
    return CDirEntry::createDir(newdir);
  }

  void removeFolder(const std::string& foldername)
  {
    if (!CDirEntry::remove(foldername))
    {
      std::vector<std::string> files = filesFromDirectory(foldername);
      std::vector<std::string>::iterator it = files.begin();
      for (; it != files.end(); ++it)
      {
        if (isDirectory(*it) && *it != foldername)
          removeFolder(*it);
        else
          std::remove(it->c_str());
      }
      CDirEntry::remove(foldername);
    }
  }

  bool isDirectory(const std::string& path)
  {
    return CDirEntry::isDir(path);
  }

  std::string parentDirectory(const std::string& filepath)
  {
    return CDirEntry::dirName(filepath);
  }

  std::string currentPath()
  {
    char buffer[1024];
    getcwd(buffer, 1024);
    std::string result(buffer);
    return result;

  }


  std::vector<std::string> filesFromDirectory(const std::string& path)
  {
    std::vector<std::string> files;
    DIR*           dir;
    struct dirent* entry;

    dir = opendir(path.c_str());

    if (dir == NULL)
    {
      return files;
    }

    for (entry = readdir(dir); entry != NULL; entry = readdir(dir))
    {
      std::string filename(entry->d_name);

      if (filename == "." || filename == "..") continue;

      if (CDirEntry::isDir(path + CDirEntry::Separator + filename))
      {
        std::vector<std::string> moreFiles = filesFromDirectory(path + CDirEntry::Separator + filename);
        std::copy(moreFiles.begin(), moreFiles.end(), std::back_inserter(files));
        continue;
      }


      files.push_back(path + CDirEntry::Separator + filename);
    }

    closedir(dir);


    return files;
  }

  std::string fileNameFromPath(const std::string& fullPath)
  {
    return CDirEntry::fileName(fullPath);
  }

}
