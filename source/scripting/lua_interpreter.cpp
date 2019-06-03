#include "scripting/lua_interpreter.hpp"

#include <iostream>
#include <algorithm>

#include "helpers/encoding.hpp"

int lua_getSaveFileBuffer(lua_State *state);
int lua_getStrArguments(lua_State *state);
int lua_getIntArguments(lua_State *state);

typedef int (LuaInterpreter::*mem_func)(lua_State *s);

template <mem_func func>
int dispatch(lua_State *s) {
  LuaInterpreter * ptr = *static_cast<LuaInterpreter**>(lua_getextraspace(s));
  return ((*ptr).*func)(s);
}

LuaInterpreter::LuaInterpreter() {
  m_luaState = nullptr;
  m_buffer.clear();
}

LuaInterpreter::~LuaInterpreter() {
  if (m_luaState != nullptr) {
    lua_close(m_luaState);
    m_luaState = nullptr;
  }

}

void LuaInterpreter::printError(lua_State *luaState) {
  printf("%s\n", lua_tostring(luaState, -1));
}

bool LuaInterpreter::initialize(std::string filetype) {
  m_luaState = luaL_newstate();

  luaL_openlibs(m_luaState);

  *static_cast<LuaInterpreter**>(lua_getextraspace(m_luaState)) = this;

  const luaL_Reg regs[] {
    { "getSaveFileBuffer", &dispatch<&LuaInterpreter::lua_getSaveFileBuffer> },
    { "getSaveFileString", &dispatch<&LuaInterpreter::lua_getSaveFileString> },
    { "getStrArgs", &dispatch<&LuaInterpreter::lua_getStrArgs> },
    { "getIntArgs", &dispatch<&LuaInterpreter::lua_getIntArgs> },
    { nullptr, nullptr }
  };

  luaL_newlib(m_luaState, regs);
  lua_setglobal(m_luaState, "edizon");

  std::string path = EDIZON_DIR "/editor/scripts/";
  path += filetype;
  path += ".lua";

  luaL_loadfile(m_luaState, path.c_str());

  if(lua_pcall(m_luaState, 0, 0, 0)) {
    printError(m_luaState);
    return false;
  }

  printf("Lua interpreter initialized!\n");

  return true;
}

void LuaInterpreter::deinitialize() {
  if (m_luaState != nullptr) {
    lua_close(m_luaState);
    m_luaState = nullptr;
  }
}

s64 LuaInterpreter::getValueFromSaveFile() {
  s64 out;

  lua_getglobal(m_luaState, "getValueFromSaveFile");
  if (lua_pcall(m_luaState, 0, 1, 0)) {
    printError(m_luaState);
    printf("Error while calling Lua function getValueFromSaveFile!\n");
  }

  out = lua_tointeger(m_luaState, -1);
  lua_pop(m_luaState, 1);

  return out;
}

s64 LuaInterpreter::getDummyValue() {
  s64 out;

  lua_getglobal(m_luaState, "getDummyValue");
  if (lua_pcall(m_luaState, 0, 1, 0)) {
    printError(m_luaState);
    printf("Error while calling Lua function getDummyValue!\n");
  }

  out = lua_tointeger(m_luaState, -1);
  lua_pop(m_luaState, 1);

  return out;
}


std::string LuaInterpreter::getStringFromSaveFile() {
  std::string out;

  lua_getglobal(m_luaState, "getStringFromSaveFile");
  if (lua_pcall(m_luaState, 0, 1, 0)) {
    printError(m_luaState);
    printf("Error while calling Lua function getStringFromSaveFile!\n");
  }

  out = lua_tostring(m_luaState, -1);
  lua_pop(m_luaState, 1);

  return out;
}

std::string LuaInterpreter::getDummyString() {
  std::string out;

  lua_getglobal(m_luaState, "getDummyString");
  if (lua_pcall(m_luaState, 0, 1, 0)) {
    printError(m_luaState);
    printf("Error while calling Lua function getDummyString!\n");
  }

  out = lua_tostring(m_luaState, -1);
  lua_pop(m_luaState, 1);

  return out;
}

void LuaInterpreter::setValueInSaveFile(s64 value) {
  lua_getglobal(m_luaState, "setValueInSaveFile");
  lua_pushinteger(m_luaState, value);
  if (lua_pcall(m_luaState, 1, 0, 0)) {
    printError(m_luaState);
    printf("Error while calling Lua function setValueInSaveFile!\n");
  }
}

void LuaInterpreter::setStringInSaveFile(std::string value) {
  lua_getglobal(m_luaState, "setStringInSaveFile");
  lua_pushstring(m_luaState, value.c_str());
  if (lua_pcall(m_luaState, 1, 0, 0)) {
    printError(m_luaState);
    printf("Error while calling Lua function setStringInSaveFile!\n");
  }
}

void LuaInterpreter::setDummyValue(s64 value) {
  lua_getglobal(m_luaState, "setDummyValue");
  lua_pushinteger(m_luaState, value);
  if (lua_pcall(m_luaState, 1, 0, 0)) {
    printError(m_luaState);
    printf("Error while calling Lua function setDummyValue!\n");
  }
}

void LuaInterpreter::setDummyString(std::string value) {
  lua_getglobal(m_luaState, "setDummyString");
  lua_pushstring(m_luaState, value.c_str());
  if (lua_pcall(m_luaState, 1, 0, 0)) {
    printError(m_luaState);
    printf("Error while calling Lua function setDummyString!\n");
  }
}

void LuaInterpreter::getModifiedSaveFile(std::vector<u8> &buffer) {
  std::vector<u8> encoded;

  lua_getglobal(m_luaState, "getModifiedSaveFile");
  if (lua_pcall(m_luaState, 0, 1, 0)) {
    printError(m_luaState);
    printf("Error while calling Lua function getModifiedSaveFile!\n");
    return;
  }

  lua_pushnil(m_luaState);

    while (lua_next(m_luaState, 1)) {
        encoded.push_back(lua_tointeger(m_luaState, -1));
        lua_pop(m_luaState, 1);
    }

  lua_pop(m_luaState, 1);

  switch (m_encoding) {
    case UTF_16BE:
      buffer = Encoding::utf8ToUtf16be(&encoded[0], encoded.size());
      break;
    case UTF_16LE:
      buffer = Encoding::utf8ToUtf16le(&encoded[0], encoded.size());
      break;
    case ASCII: [[fallthrough]]
    case UTF_8: [[fallthrough]]
    default:
      buffer = encoded;
      break;
  }
}

std::string LuaInterpreter::callFunction(std::string funcName) {
  lua_getglobal(m_luaState, funcName.c_str());
  if (lua_pcall(m_luaState, 0, 1, 0)) {
    printError(m_luaState);
    printf("Error while calling Lua function %s!\n", funcName.c_str());
  }

  std::string out = lua_tostring(m_luaState, -1);
  lua_pop(m_luaState, 1);

  return out;
} 

int LuaInterpreter::lua_getSaveFileBuffer(lua_State *state) {
  lua_createtable(state, m_bufferSize, 0);

  for (u64 i = 0; i < m_bufferSize; i++) {
    lua_pushinteger(state, i + 1);
    lua_pushinteger(state, m_buffer[i]);
    lua_settable(state, -3);
  }

  return 1;
}

int LuaInterpreter::lua_getSaveFileString(lua_State *state) {
  std::string str = reinterpret_cast<char*>(&m_buffer[0]);

  str += '\x00';

  str[m_bufferSize] = 0x00;

  lua_pushstring(state, str.c_str());

  return 1;
}

int LuaInterpreter::lua_getStrArgs(lua_State *state) {
  lua_createtable(state, m_strArgs.size(), 0);

  for (u64 i = 0; i < m_strArgs.size(); i++) {
    lua_pushinteger(state, i + 1);
    lua_pushstring(state, m_strArgs[i].c_str());
    lua_settable(state, -3);
  }

  return 1;
}

int LuaInterpreter::lua_getIntArgs(lua_State *state) {
  lua_createtable(state, m_intArgs.size(), 0);

  for (u64 i = 0; i < m_intArgs.size(); i++) {
    lua_pushinteger(state, i + 1);
    lua_pushinteger(state, m_intArgs[i]);
    lua_settable(state, -3);
  }

  return 1;
}
