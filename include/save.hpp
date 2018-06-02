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

Result _getSaveList(std::vector<FsSaveDataInfo> &saveInfoList);

void makeExInjDir(char ptr[0x100], u64 titleID, u128 userID, bool isInject);
Result mountSaveByTitleAccountIDs(const u64 titleID, const u128 userID, FsFileSystem& tmpfs);
bool getSavefilesForGame(std::vector<int>& vec, u64 titleID, u128 userID);
int isDirectory(const char *path);
int cpFile(std::string srcPath, std::string dstPath);
int copyAllSave(const char * path, bool isInject, const char exInjDir[0x100]);

int backupSave(u64 titleID, u128 userID);
int restoreSave(u64 titleID, u128 userID, const char* injectFolder);
