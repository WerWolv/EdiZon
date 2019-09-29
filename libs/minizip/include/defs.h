#pragma once

extern "C"
{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

#if (defined(_WIN32)) || (defined(_WIN64))
# include <direct.h>
# include <io.h>
#else
# include <unistd.h>
# include <utime.h>
#endif

#include <zip.h>
#include <unzip.h>
#include <ioapi_mem.h>
#define CASESENSITIVITY (0)
#define WRITEBUFFERSIZE (8192)
#define MAXFILENAME (256)

#if (defined(_WIN32)) || (defined(_WIN64))
#define USEWIN32IOAPI
#include "iowin32.h"
#endif
}

#if (defined(_WIN32)) || (defined(_WIN64))
  #include <filesystem>
#endif

#if (defined(_WIN32)) || (defined(_WIN64))
    #define EXCEPTION_CLASS std::exception
#else
    #define EXCEPTION_CLASS std::runtime_error
#endif


#if (defined(_WIN64)) && (!defined(__APPLE__))
#ifndef __USE_FILE_OFFSET64
#define __USE_FILE_OFFSET64
#endif
#ifndef __USE_LARGEFILE64
#define __USE_LARGEFILE64
#endif
#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif
#ifndef _FILE_OFFSET_BIT
#define _FILE_OFFSET_BIT 64
#endif
#endif

#if (defined(_WIN32)) || (defined(_WIN64))
#  define MKDIR(d) _mkdir(d)
#  define CHDIR(d) _chdir(d)
#else
#  define MKDIR(d) mkdir(d, 0775)
#  define CHDIR(d) chdir(d)
#endif
