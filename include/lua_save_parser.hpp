#pragma once

#define LUA_C89_NUMBERS
#include "lua.hpp"

#include <string>
#include <vector>

#include <switch.h>

class LuaSaveParser {
public:
  LuaSaveParser(std::string filetype, u8 *buffer, size_t bufferSize);
  ~LuaSaveParser();

  void getValueFromSaveFile(u64 location, u64 &out);
  void getValueFromSaveFile(std::string location, u64 &out);
  void getValueFromSaveFile(u64 location, std::string &out);
  void getValueFromSaveFile(std::string location, std::string &out);

  void setValueInSaveFile(u64 location, u64 value);
  void setValueInSaveFile(std::string location, u64 value);
  void setValueInSaveFile(u64 location, std::string value);
  void setValueInSaveFile(std::string location, std::string value);

  void setLuaArguments(std::vector<u64> intArguments, std::vector<std::string> strArguments);

  int lua_getSaveFileBuffer(lua_State *state);
  int lua_getStrArguments(lua_State *state);
  int lua_getIntArguments(lua_State *state);

private:
  std::string m_filetype;
  u8 *m_buffer;
  size_t m_bufferSize;

  lua_State *m_luaState;
  std::vector<u64> m_intArguments;
  std::vector<std::string> m_strArguments;
};
