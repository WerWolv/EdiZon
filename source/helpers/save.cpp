#include "helpers/save.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "helpers/account.hpp"
#include "helpers/title.hpp"

using json = nlohmann::json;

s32 deleteDirRecursively(const char *path, bool isSave) {
  DIR *d = opendir(path);
     size_t path_len = strlen(path);
     s32 r = -1;

     if (d) {
        struct dirent *p;

        r = 0;

        while (!r && (p=readdir(d))) {
            s32 r2 = -1;
            char *buf;
            size_t len;

            /* Skip the names "." and ".." as we don't want to recurse on them. */
            if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, "..")) {
               continue;
            }

            len = path_len + strlen(p->d_name) + 2;
            buf = new char[len];

            if (buf) {
               struct stat statbuf;

               snprintf(buf, len, "%s/%s", path, p->d_name);

               if (!stat(buf, &statbuf)) {
                  if (S_ISDIR(statbuf.st_mode))
                     r2 = deleteDirRecursively(buf, isSave);
                  else
                     r2 = unlink(buf);
               }

               delete[] buf;
            }

            r = r2;
        }

        closedir(d);
     }

     if (!r)
        r = rmdir(path);


     if (isSave && R_FAILED(fsdevCommitDevice(SAVE_DEV))) {
       printf("Committing failed.\n");
       return -3;
     }

     return r;
}

bool doesFolderExist(const char *path) {
  struct stat sb;
  
  return stat(path, &sb) == 0 && S_ISDIR(sb.st_mode);
}

bool makeExInjDir(char ptr[0x100], u64 titleID, u128 userID, bool isInject, const char* injectFolder, bool fromBatch, std::string backupName) {
  std::stringstream ss;
  std::string folder_path(EDIZON_DIR);

  json metadata;
  std::string metadata_string;
  std::ofstream metadata_file;
  std::stringstream metadata_user_id;
  std::stringstream metadata_title_id;

  if (!fromBatch) {
    folder_path += "/saves/";
    mkdir(folder_path.c_str(), 0700);
    ss << folder_path << std::uppercase << std::setfill('0') << std::setw(sizeof(titleID)*2)
     << std::hex << titleID << "/";
    mkdir(ss.str().c_str(), 0700);

    std::string titleName = ss.str();
    titleName += Title::g_titles[titleID]->getTitleName();
    fclose(fopen(titleName.c_str(), "ab+"));

    if (isInject)
      ss << injectFolder << "/";
    else
      ss << backupName << "/";
  }
  else {
    folder_path += "/batch_saves/";
    mkdir(folder_path.c_str(), 0700);
    ss << folder_path << backupName << "/";


    mkdir(ss.str().c_str(), 0700);

    u64 userIDH = userID >> 64;
    u64 userIDL = userID & 0xFFFFFFFFFFFFFFFFULL;

    ss << std::setfill('0') << std::uppercase << std::hex << userIDH << std::setfill('0') << std::uppercase << std::hex << userIDL << "/";
    mkdir(ss.str().c_str(), 0700);

    ss << std::uppercase << std::setfill('0') << std::setw(sizeof(titleID)*2)
      << std::hex << titleID << "/";
  }

  if(doesFolderExist(ss.str().c_str())) return false;
  strcpy(ptr, ss.str().c_str());
  mkdir(ptr, 0700);

  metadata.clear();

  u64 userIDH = userID >> 64;
  u64 userIDL = userID & 0xFFFFFFFFFFFFFFFFULL;
  metadata_user_id  << std::setfill('0') << std::uppercase << std::hex << userIDH;
  metadata_user_id  << std::setfill('0') << std::uppercase << std::hex << userIDL;
  metadata["user_id"] = metadata_user_id.str();

  metadata["user_name"] = Account::g_accounts[userID]->getUserName();

  metadata_title_id << std::uppercase << std::setfill('0') << std::setw(sizeof(titleID)*2) << std::hex << titleID;
  metadata["title_id"] = metadata_title_id.str();

  metadata["title_name"] = Title::g_titles[titleID]->getTitleName();
  metadata["title_version"] = Title::g_titles[titleID]->getTitleVersion();
  metadata_string = metadata.dump(4);


  metadata_file.open (ss.str() + "edizon_save_metadata.json");
  metadata_file << metadata_string << "\n";
  metadata_file.close();

  return true;
}

Result _getSaveList(std::vector<FsSaveDataInfo> & saveInfoList) {
  Result rc=0;
  FsSaveDataIterator iterator;
  size_t total_entries=0;
  FsSaveDataInfo info;

  rc = fsOpenSaveDataIterator(&iterator, FsSaveDataSpaceId_NandUser);//See libnx fs.h.
  if (R_FAILED(rc)) {
    printf("fsOpenSaveDataIterator() failed: 0x%x\n", rc);
    return rc;
  }

  rc = fsSaveDataIteratorRead(&iterator, &info, 1, &total_entries);//See libnx fs.h.
  if (R_FAILED(rc))
    return rc;
  if (total_entries == 0)
    return MAKERESULT(Module_Libnx, LibnxError_NotFound);

  for (; R_SUCCEEDED(rc) && total_entries > 0;
    rc = fsSaveDataIteratorRead(&iterator, &info, 1, &total_entries)) {
    if (info.SaveDataType == FsSaveDataType_SaveData) {
      saveInfoList.push_back(info);
    }
  }

  fsSaveDataIteratorClose(&iterator);

  return 0;
}

Result mountSaveByTitleAccountIDs(const u64 titleID, const u128 userID, FsFileSystem& tmpfs) {
  Result rc = 0;

  rc = fsMount_SaveData(&tmpfs, titleID, userID);//See also libnx fs.h.
  if (R_FAILED(rc)) {
    printf("fsMount_SaveData() failed: 0x%x\n", rc);
    fsdevUnmountDevice(SAVE_DEV);
    fsFsClose(&tmpfs);
    return rc;
  }

  s32 ret = fsdevMountDevice(SAVE_DEV, tmpfs);
  if (ret == -1) {
    printf("fsdevMountDevice() failed.\n");
    rc = ret;
  }
  return rc;
}

s32 isDirectory(const char *path) {
 struct stat statbuf;

 if (stat(path, &statbuf) != 0)
   return 0;

 return S_ISDIR(statbuf.st_mode);
}

s32 cpFile(std::string srcPath, std::string dstPath) {
  FILE* src = fopen(srcPath.c_str(), "rb");
  FILE* dst = fopen(dstPath.c_str(), "wb+");

  if (src == nullptr || dst == nullptr)
      return - 1;

  fseek(src, 0, SEEK_END);
  rewind(src);

  size_t size;
  char* buf = new char[0x50000];

  u64 offset = 0;
  size_t slashpos = srcPath.rfind("/");
  std::string name = srcPath.substr(slashpos + 1, srcPath.length() - slashpos - 1);
  while ((size = fread(buf, 1, 0x50000, src)) > 0) {
      fwrite(buf, 1, size, dst);
      offset += size;
  }

  delete[] buf;
  fclose(src);
  fclose(dst);

  return 0;
}

s32 copyAllSave(const char * path, bool isInject, const char exInjDir[0x100]) {
  DIR* dir;
  struct dirent* ent;

  char filenameSave[0x100];
  char filenameSD[0x100];

  strcpy(filenameSave, "save:/");
  strcat(filenameSave, path);

  strcpy(filenameSD, exInjDir);
  strcat(filenameSD, path);

  if (isInject)
    dir = opendir(filenameSD);
  else
    dir = opendir(filenameSave);

  if (dir == nullptr) {
    printf("Failed to open dir: %s\n", isInject ? filenameSD : filenameSave);
    return -1;
  }
  else {
    while ((ent = readdir(dir))) {
      if (strcmp(ent->d_name, "edizon_save_metadata.json") == 0) continue;

      char filename[0x100];

      strcpy(filename, path);
      strcat(filename, "/");
      strcat(filename, ent->d_name);

      strcpy(filenameSave, "save:/");
      strcat(filenameSave, filename);

      strcpy(filenameSD, exInjDir);
      strcat(filenameSD, filename);

      if (isDirectory(isInject ? filenameSD : filenameSave)) {
          if (isInject) {
            mkdir(filenameSave, 0700);
            if (R_FAILED(fsdevCommitDevice(SAVE_DEV)))
              printf("Failed to commit directory %s.", filenameSave);
          } else
            mkdir(filenameSD, 0700);
          s32 res = copyAllSave(filename, isInject, exInjDir);
          if (res != 0)
              return res;
      } else {
        printf("Copying %s... ", filename);

        if (isInject) {
          cpFile(std::string(filenameSD), std::string(filenameSave));

          if (R_SUCCEEDED(fsdevCommitDevice(SAVE_DEV))) { // Thx yellows8
              printf("committed.\n");
          } else {
              printf("fsdevCommitDevice() failed...\n");
              return -2;
          }
        } else {
          cpFile(std::string(filenameSave), std::string(filenameSD));
          printf("\n");
        }
      }
    }
    closedir(dir);
    return 0;
  }
}

s32 backupSave(u64 titleID, u128 userID, bool fromBatch, std::string backupName) {
  FsFileSystem fs;
  s32 res = 0;

  if (R_FAILED(mountSaveByTitleAccountIDs(titleID, userID, fs))) {
    printf("Failed to mount save.\n");
    return 1;
  }

  char *ptr = new char[0x100];

  if(!makeExInjDir(ptr, titleID, userID, false, nullptr, fromBatch, backupName)) {
    fsdevUnmountDevice(SAVE_DEV);
    delete[] ptr;
    return 2;
  }

  if (ptr == nullptr) {
    printf("makeExInjDir failed.\n");
    fsdevUnmountDevice(SAVE_DEV);
    delete[] ptr;
    return 3;
  }

  res = copyAllSave("", false, ptr);
  fsdevUnmountDevice(SAVE_DEV);

  delete[] ptr;

  return res;
}

s32 restoreSave(u64 titleID, u128 userID, const char* path) {
  FsFileSystem fs;
  s32 res = 0;

  if (R_FAILED(mountSaveByTitleAccountIDs(titleID, userID, fs))) {
    printf("Failed to mount save.\n");
    return 1;
  }

  if (path == nullptr) {
    printf("makeExInjDir failed.\n");
    fsdevUnmountDevice(SAVE_DEV);
    return 2;
  }

  res = deleteDirRecursively("save:/", true);

  if (!res) {
    printf("Deleting save:/ failed: %d.\n", res);
    return 3;
  }

  res = copyAllSave("", true, path);
  fsdevUnmountDevice(SAVE_DEV);
  fsFsClose(&fs);

  return res;
}

s32 loadSaveFile(std::vector<u8> *buffer, size_t *length, u64 titleID, u128 userID, const char *path) {
  FsFileSystem fs;
  size_t size;

  if (R_FAILED(mountSaveByTitleAccountIDs(titleID, userID, fs))) {
    printf("Failed to mount save.\n");
    fsdevUnmountDevice(SAVE_DEV);
    fsFsClose(&fs);
    return -1;
  }

  char filePath[0x100];

  strcpy(filePath, "save:/");
  strcat(filePath, path);


  FILE *file = fopen(filePath, "rb");

  if (file == nullptr) {
    printf("Failed to open file.\n");
    fsdevUnmountDevice(SAVE_DEV);
    fsFsClose(&fs);
    return -2;
  }

  fseek(file, 0, SEEK_END);
  size = ftell(file);
  rewind(file);

  if (size <= 0) {
    printf("File reading failed. File length is %zu.\n", size);
    fclose(file);
    fsdevUnmountDevice(SAVE_DEV);
    return -3;
  }

  buffer->reserve(size);
  fread(&(*buffer)[0], size, 1, file);
  fclose(file);

  *length = size;

  fsdevUnmountDevice(SAVE_DEV);
  fsFsClose(&fs);

  return 0;
}

s32 storeSaveFile(u8 *buffer, size_t length, u64 titleID, u128 userID, const char *path) {
  FsFileSystem fs;

  if (R_FAILED(mountSaveByTitleAccountIDs(titleID, userID, fs))) {
    printf("Failed to mount save.\n");
    fsdevUnmountDevice(SAVE_DEV);
    fsFsClose(&fs);
    return -1;
  }

  char filePath[0x100];

  strcpy(filePath, "save:/");
  strcat(filePath, path);


  FILE *file = fopen(filePath, "wb");

  if (file == nullptr) {
    printf("Failed to open file.\n");
    fsdevUnmountDevice(SAVE_DEV);
    fsFsClose(&fs);
    return -2;
  }

  fwrite(buffer, length, 1, file);
  fclose(file);

  if (R_FAILED(fsdevCommitDevice(SAVE_DEV))) {
    printf("Committing failed.\n");
    return -3;
  }

  fsdevUnmountDevice(SAVE_DEV);
  fsFsClose(&fs);

  return 0;
}
