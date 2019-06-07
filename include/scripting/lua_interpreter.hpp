#pragma once

#define LLONG_MAX INT64_MAX
#define LLONG_MIN INT64_MIN
#include "lua.hpp"

#include "interpreter.hpp"

#include <string>
#include <vector>

#include <edizon.h>

class LuaInterpreter : public Interpreter{
public:
  LuaInterpreter();
  ~LuaInterpreter();

  bool initialize(std::string filetype);
  void deinitialize();

  s64 getValueFromSaveFile();
  std::string getStringFromSaveFile();

  void setValueInSaveFile(s64 value);
  void setStringInSaveFile(std::string value);

  void getModifiedSaveFile(std::vector<u8> &buffer);

  s64 getDummyValue();
  std::string getDummyString();
  void setDummyValue(s64 value);
  void setDummyString(std::string value);

  std::string callFunction(std::string funcName);

  int lua_getSaveFileBuffer(lua_State *state);
  int lua_getSaveFileString(lua_State *state);
  int lua_getStrArgs(lua_State *state);
  int lua_getIntArgs(lua_State *state);

private:
  std::string m_filetype;

  lua_State *m_luaState;

  enum {
    ASCII,
    UTF_8,
    UTF_16LE,
    UTF_16BE
  } m_encoding = ASCII;

  static void printError(lua_State *luaState);
};
