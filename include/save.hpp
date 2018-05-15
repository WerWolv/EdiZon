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

int dumpToTitleUserDir(FsSaveDataInfo info, bool isInject);
Result getSaveList(std::vector<FsSaveDataInfo> & saveInfoList);
Result getUserNameById(u128 userID, char * username);
bool getTitleIcon(u64 titleID, u8* decodedptr);
