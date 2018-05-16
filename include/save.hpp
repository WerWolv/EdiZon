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
Result _getSaveList(std::vector<FsSaveDataInfo> & saveInfoList);
Result _getUserNameById(u128 userID, char * username);
bool _getTitleIcon(u64 titleID, uint8_t** decodedptr);
Result _getTitleName(u64 titleID, char * name);
