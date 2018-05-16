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

Result mountSaveBySaveDataInfo(const FsSaveDataInfo & info, const char * dev) {
    Result rc=0;
    int ret=0;

    u64 titleID = info.titleID;
    u128 userID = info.userID;

    FsFileSystem tmpfs;

    printf("\n\nUsing titleID=0x%016lx userID: 0x%lx 0x%lx\n", titleID, (u64)(userID>>64), (u64)userID);

    rc = fsMount_SaveData(&tmpfs, titleID, userID);//See also libnx fs.h.
    if (R_FAILED(rc)) {
        printf("fsMount_SaveData() failed: 0x%x\n", rc);
        return rc;
    }

    ret = fsdevMountDevice(dev, tmpfs);
    if (ret==-1) {
        printf("fsdevMountDevice() failed.\n");
        rc = ret;
        return rc;
    }

    return rc;
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

bool _getTitleIcon(u64 titleID, uint8_t** decodedptr)
{
  Result rc=0;
  size_t outsize = 0;

  NsApplicationControlData* buf = (NsApplicationControlData*)malloc(sizeof(NsApplicationControlData));
  if (buf==NULL) {
      return false;
  }
  memset(buf, 0, sizeof(NsApplicationControlData));

  if (R_SUCCEEDED(rc)) {
      rc = nsInitialize();
      if (R_FAILED(rc)) {
          return false;
      }
      rc = nsGetApplicationControlData(1, titleID, buf, sizeof(NsApplicationControlData), &outsize);
      if (R_FAILED(rc)) {
          return false;
      }

      if (outsize < sizeof(buf->nacp)) {
          return false;
      }
  }

  njInit();

  if (njDecode(buf->icon, outsize-sizeof(buf->nacp)) != NJ_OK)
  {
      njDone();
      return false;
  }

  if (njGetWidth() != 256 || njGetHeight() != 256 || (size_t)njGetImageSize() != 256*256*3)
  {
      njDone();
      return false;
  }

  *decodedptr = njGetImage();
  if (*decodedptr == NULL)
  {
      njDone();
      return false;
  }

  njDone();
  return true;
}

Result _getTitleName(u64 titleID, char * name) {
    Result rc=0;

    NsApplicationControlData *buf=NULL;
    size_t outsize=0;

    NacpLanguageEntry *langentry = NULL;

    buf = (NsApplicationControlData*)malloc(sizeof(NsApplicationControlData));
    if (buf==NULL) {
        rc = MAKERESULT(Module_Libnx, LibnxError_OutOfMemory);
        printf("Failed to alloc mem.\n");
    }
    else {
        memset(buf, 0, sizeof(NsApplicationControlData));
    }

    if (R_SUCCEEDED(rc)) {
        rc = nsInitialize();
        if (R_FAILED(rc)) {
            printf("nsInitialize() failed: 0x%x\n", rc);
        }
    }

    if (R_SUCCEEDED(rc)) {
        rc = nsGetApplicationControlData(1, titleID, buf, sizeof(NsApplicationControlData), &outsize);
        if (R_FAILED(rc)) {
            printf("nsGetApplicationControlData() failed: 0x%x\n", rc);
        }

        if (outsize < sizeof(buf->nacp)) {
            rc = -1;
            printf("Outsize is too small: 0x%lx.\n", outsize);
        }

        if (R_SUCCEEDED(rc)) {
            rc = nacpGetLanguageEntry(&buf->nacp, &langentry);

            if (R_FAILED(rc) || langentry==NULL) printf("Failed to load LanguageEntry.\n");
        }

        if (R_SUCCEEDED(rc)) {
            memset(name, 0, sizeof(*name));
            strncpy(name, langentry->name, sizeof(langentry->name));//Don't assume the nacp string is NUL-terminated for safety.
        }

        nsExit();
    }

    return rc;
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
