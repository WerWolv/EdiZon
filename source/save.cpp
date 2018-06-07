#include "save.hpp"

#include <unistd.h>

#include "account.hpp"
extern "C" {
#include "nanojpeg.h"
}



const char* ROOT_DIR = "/EdiZon/";
const char* SAVE_DEV = "save";

s32 deleteDirRecursively(const char *path, bool isSave) {
  DIR *d = opendir(path);
     size_t path_len = strlen(path);
     int r = -1;

     if (d)
     {
        struct dirent *p;

        r = 0;

        while (!r && (p=readdir(d)))
        {
            int r2 = -1;
            char *buf;
            size_t len;

            /* Skip the names "." and ".." as we don't want to recurse on them. */
            if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, ".."))
            {
               continue;
            }

            len = path_len + strlen(p->d_name) + 2;
            buf = new char[len];

            if (buf)
            {
               struct stat statbuf;

               snprintf(buf, len, "%s/%s", path, p->d_name);

               if (!stat(buf, &statbuf))
               {
                  if (S_ISDIR(statbuf.st_mode))
                  {
                     r2 = deleteDirRecursively(buf, isSave);
                  }
                  else
                  {
                     r2 = unlink(buf);
                  }
               }

               delete[] buf;
            }

            r = r2;
        }

        closedir(d);
     }

     if (!r)
     {
        r = rmdir(path);
     }


     if (isSave && R_FAILED(fsdevCommitDevice(SAVE_DEV)))
     {
       printf("Committing failed.\n");
       return -3;
     }

     return r;
}

void makeExInjDir(char ptr[0x100], u64 titleID, u128 userID, bool isInject, const char* injectFolder)
{
  time_t t = time(nullptr);
  std::stringstream ss;

  mkdir(ROOT_DIR, 0700);

  ss << ROOT_DIR << std::setfill('0') << std::setw(sizeof(titleID)*2)
     << std::hex << titleID << "/";
  mkdir(ss.str().c_str(), 0700);

  if (isInject)
    ss << injectFolder << "/";
  else {
    ss << std::put_time(std::gmtime(&t), "%Y%m%d%H%M%S") << "_" << Account::g_accounts[userID]->getUserName() << "/";
  }


  strcpy(ptr, ss.str().c_str());
  mkdir(ptr, 0700);
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
  if (total_entries==0)
    return MAKERESULT(Module_Libnx, LibnxError_NotFound);

  for(; R_SUCCEEDED(rc) && total_entries > 0;
    rc = fsSaveDataIteratorRead(&iterator, &info, 1, &total_entries)) {
    if (info.SaveDataType == FsSaveDataType_SaveData) {
      saveInfoList.push_back(info);
    }
  }

  fsSaveDataIteratorClose(&iterator);

  return 0;
}

Result mountSaveByTitleAccountIDs(const u64 titleID, const u128 userID, FsFileSystem& tmpfs)
{
  Result rc = 0;

  printf("\n\nUsing titleID=0x%016lx userID: 0x%lx 0x%lx\n", titleID, (u64)(userID>>64), (u64)userID);

  rc = fsMount_SaveData(&tmpfs, titleID, userID);//See also libnx fs.h.
  if (R_FAILED(rc)) {
    printf("fsMount_SaveData() failed: 0x%x\n", rc);
    return rc;
  }

  s32 ret = fsdevMountDevice(SAVE_DEV, tmpfs);
  if (ret==-1) {
    printf("fsdevMountDevice() failed.\n");
    rc = ret;
  }
  return rc;
}

bool getSavefilesForGame(std::vector<s32>& vec, u64 titleID, u128 userID)
{
  FsFileSystem tmpfs;
  Result rc = 0;
  if (titleID != 0x0100000000010000)
    return false;

  rc = mountSaveByTitleAccountIDs(titleID, userID, tmpfs);
  if (R_FAILED(rc))
    return false;

  char fname[0x10];
  struct stat statbuf;
  s32 i;
  for (i=0; i != 7; i++)
  {
    sprintf(fname, "File%d.bin", i);
    if (stat(fname, &statbuf) != 0)
      break;
    vec.push_back(i);
  }
  if (i == 0)
    return false;
  return true;
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

  if(isInject)
    dir = opendir(filenameSD);
  else
    dir = opendir(filenameSave);

  if(dir==NULL)
  {
    printf("Failed to open dir: %s\n", isInject ? filenameSD : filenameSave);
    return -1;
  }
  else
  {
    while ((ent = readdir(dir)))
    {
      char filename[0x100];
      strcpy(filename, path);
      strcat(filename, "/");
      strcat(filename, ent->d_name);

      strcpy(filenameSave, "save:/");
      strcat(filenameSave, filename);

      strcpy(filenameSD, exInjDir);
      strcat(filenameSD, filename);

      if(isDirectory(isInject ? filenameSD : filenameSave)) {
          if (isInject)
          {
            mkdir(filenameSave, 0700);
            if (R_FAILED(fsdevCommitDevice(SAVE_DEV)))
              printf("Failed to commit directory %s.", filenameSave);
          } else
            mkdir(filenameSD, 0700);
          s32 res = copyAllSave(filename, isInject, exInjDir);
          if(res != 0)
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

s32 backupSave(u64 titleID, u128 userID) {
  FsFileSystem fs;
  char *ptr = new char[0x100];
  s32 res = 0;

  if (R_FAILED(mountSaveByTitleAccountIDs(titleID, userID, fs))) {
    printf("Failed to mount save.\n");
    return 1;
  }

  makeExInjDir(ptr, titleID, userID, false, nullptr);

  if(ptr == nullptr) {
    printf("makeExInjDir failed.\n");
      fsdevUnmountDevice("save");
      return 1;
  }

  res = copyAllSave("", false, ptr);
  fsdevUnmountDevice("save");

  delete[] ptr;

  return res;
}

s32 restoreSave(u64 titleID, u128 userID, const char* injectFolder) {
  FsFileSystem fs;
  char *ptr = new char[0x100];
  s32 res = 0;

  if (R_FAILED(mountSaveByTitleAccountIDs(titleID, userID, fs))) {
    printf("Failed to mount save.\n");
    return 1;
  }

  makeExInjDir(ptr, titleID, userID, true, injectFolder);

  if(ptr == nullptr) {
    printf("makeExInjDir failed.\n");
      fsdevUnmountDevice("save");
      return 1;
  }
  res = deleteDirRecursively("save:/", true);
  if (!res)
  {
    printf("Deleting save:/ failed: %d.\n", res);
    return res;
  }
  res = copyAllSave("", true, ptr);
  fsdevUnmountDevice("save");
  fsFsClose(&fs);

  delete[] ptr;

  return res;
}

s32 loadSaveFile(u8 **buffer, size_t *length, u64 titleID, u128 userID, const char *path) {
  FsFileSystem fs;
  size_t size;

  if (R_FAILED(mountSaveByTitleAccountIDs(titleID, userID, fs))) {
    printf("Failed to mount save.\n");
    return 1;
  }

  char filePath[0x100];

  strcpy(filePath, "save:/");
  strcat(filePath, path);


  FILE *file = fopen(filePath, "rb");

  if(file == nullptr) {
    printf("Failed to open file.\n");
    return 2;
  }

  fseek(file, 0, SEEK_END);
  size = ftell(file);
  rewind(file);

  if(size <= 0) {
    printf("File reading failed. File length is %d.", size);
    return 3;
  }

  *buffer = new u8[size];
  fread(*buffer, size, 1, file);
  fclose(file);

  *length = size;

  fsdevUnmountDevice("save");
  fsFsClose(&fs);

  return 0;
}

u16 getValueFromAddressAtOffset(u8 *buffer, u32 offsetAddress, u32 address) {
  u16 offset = *((u16*)(buffer + offsetAddress));

  return *((u16*)(buffer + offset + address));
}

void setValueAtAddressAtOffset(u8 *buffer, u32 offsetAddress, u32 address, u16 value) {
  u16 offset = *((u16*)(buffer + offsetAddress));

  *(((u16*)buffer) + offset + address) = value;
}
