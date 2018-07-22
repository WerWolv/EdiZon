#pragma once

#define LUA_C89_NUMBERS
#include "lua.hpp"

#include <string>
#include <vector>

#include <switch.h>

class LuaSaveParser {
public:
  LuaSaveParser();
  ~LuaSaveParser();

  void luaInit(std::string filetype);
  void luaDeinit();

  s32 getValueFromSaveFile();
  std::string getStringFromSaveFile();
  void setValueInSaveFile(s32 value);
  void setStringInSaveFile(std::string value);
  void getModifiedSaveFile(std::vector<u8> &buffer, size_t *outSize);

  void setLuaArgs(std::vector<s32> intArgs, std::vector<std::string> strArgs);
  void setLuaSaveFileBuffer(u8 *buffer, size_t bufferSize);

  int lua_getSaveFileBuffer(lua_State *state);
  int lua_getSaveFileString(lua_State *state);
  int lua_getStrArgs(lua_State *state);
  int lua_getIntArgs(lua_State *state);

private:
  std::string m_filetype;
  u8 *m_buffer;
  size_t m_bufferSize;

  lua_State *m_luaState;
  std::vector<s32> m_intArgs;
  std::vector<std::string> m_strArgs;

  void printError(lua_State *luaState);
};
