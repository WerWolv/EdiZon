#pragma once

#include <cstring>
#include <cstdio>
#include <stdlib.h>
#include <dirent.h>
#include <fstream>
#include <vector>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <switch.h>

int _dumpToTitleUserDir(FsSaveDataInfo info, bool isInject);
Result _getSaveList(std::vector<FsSaveDataInfo> &saveInfoList);
Result _getUserNameById(u128 userID, char *username);
bool _getTitleIcon(u64 titleID, u8 **decodedptr);
Result _getTitleName(u64 titleID, char *name);

void makeExInjDir(char ptr[0x100], u64 titleID, bool isInject);
Result mountSaveByTitleAccountIDs(const u64 titleID, const u128 userID, FsFileSystem& tmpfs);
bool getSavefilesForGame(std::vector<int>& vec, u64 titleID, u128 userID);
int isDirectory(const char *path);
int cpFile(std::string srcPath, std::string dstPath);
int copyAllSave(const char * path, bool isInject, const char exInjDir[0x100]);
