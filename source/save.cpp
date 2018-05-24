#include "save.hpp"
extern "C" {
#include "nanojpeg.h"
}

const char* ROOT_DIR = "/EdiZon/";
const char* SAVE_DEV = "save";

void makeExInjDir(char ptr[0x100], u64 titleID, bool isInject)
{
  time_t t = time(nullptr);
  std::stringstream ss;

  ss << ROOT_DIR << std::setfill('0') << std::setw(sizeof(titleID)*2)
     << std::hex << titleID << "/";
  mkdir(ss.str().c_str(), 0700);

  if (isInject)
    ss << "inject/";
  else
    ss << std::put_time(std::gmtime(&t), "%Y%m%d%H%M%S") << "/";

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

  int ret = fsdevMountDevice(SAVE_DEV, tmpfs);
  if (ret==-1) {
    printf("fsdevMountDevice() failed.\n");
    rc = ret;
  }
  return rc;
}

Result mountSaveBySaveDataInfo(const FsSaveDataInfo & info) {
  FsFileSystem tmpfs;
  return mountSaveByTitleAccountIDs(info.titleID, info.userID, tmpfs);
}

bool getSavefilesForGame(std::vector<int>& vec, u64 titleID, u128 userID)
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
  int i;
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

int isDirectory(const char *path) {
 struct stat statbuf;
 if (stat(path, &statbuf) != 0)
   return 0;
 return S_ISDIR(statbuf.st_mode);
}

int cpFile(const char * filenameI, const char * filenameO) {
  remove( filenameO );

  std::ifstream src(filenameI, std::ios::binary);
  std::ofstream dst(filenameO, std::ios::binary);

  dst << src.rdbuf();

  return 0;
}

int copyAllSave(const char * path, bool isInject, const char exInjDir[0x100]) {
  DIR* dir;
  struct dirent* ent;

  char filenameSave[0x100];
  char filenameSD[0x100];

  strcpy(filenameSave, "save:/");
  strcat(filenameSave, path);

  strcpy(filenameSD, exInjDir);
  strcat(filenameSD, path);

  dir = opendir(filenameSave);
  if(dir==NULL)
  {
    printf("Failed to open dir: %s\n", filenameSave);
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

      if(isDirectory(filenameSave)) {
          mkdir(filenameSD, 0700);
          int res = copyAllSave(filename, isInject, exInjDir);
          if(res != 0)
              return res;
      } else {
        printf("Copying %s... ", filename);

        if (isInject) {
          cpFile(filenameSD, filenameSave);
          if (R_SUCCEEDED(fsdevCommitDevice(SAVE_DEV))) { // Thx yellows8
              printf("committed.\n");
          } else {
              printf("fsdevCommitDevice() failed...\n");
              return -2;
          }
        } else {
          cpFile(filenameSave, filenameSD);
          printf("\n");
        }
      }
    }
    closedir(dir);
    return 0;
  }
}

int _dumpToTitleUserDir(FsSaveDataInfo info, bool isInject) {
  char exInjDir[0x100];
  makeExInjDir(exInjDir, info.titleID, isInject);
  return copyAllSave(".", isInject, exInjDir);
}

Result _getUserNameById(u128 userID, char * username) {
  Result rc=0;

  AccountProfile profile;
  AccountUserData userdata;
  AccountProfileBase profilebase;

  memset(&userdata, 0, sizeof(userdata));
  memset(&profilebase, 0, sizeof(profilebase));

  rc = accountInitialize();
  if (R_FAILED(rc)) {
      printf("accountInitialize() failed: 0x%x\n", rc);
  }

  if (R_SUCCEEDED(rc)) {
      rc = accountGetProfile(&profile, userID);

      if (R_FAILED(rc)) {
          printf("accountGetProfile() failed: 0x%x\n", rc);
      }


      if (R_SUCCEEDED(rc)) {
          rc = accountProfileGet(&profile, &userdata, &profilebase);//userdata is otional, see libnx acc.h.

          if (R_FAILED(rc)) {
              printf("accountProfileGet() failed: 0x%x\n", rc);
          }

          if (R_SUCCEEDED(rc)) {
              memset(username,  0, sizeof(*username));
              strncpy(username, profilebase.username, sizeof(profilebase.username));//Even though profilebase.username usually has a NUL-terminator, don't assume it does for safety.
          }
          accountProfileClose(&profile);
      }
      accountExit();
  }

  return rc;
}
