#include "lua_save_parser.hpp"

#include <iostream>

int lua_getSaveFileBuffer(lua_State *state);
int lua_getStrArguments(lua_State *state);
int lua_getIntArguments(lua_State *state);

typedef int (LuaSaveParser::*mem_func)(lua_State *s);

template <mem_func func>
int dispatch(lua_State *s) {
    LuaSaveParser * ptr = *static_cast<LuaSaveParser**>(lua_getextraspace(s));
    return ((*ptr).*func)(s);
}

LuaSaveParser::LuaSaveParser() {

}

LuaSaveParser::~LuaSaveParser() {
  lua_close(m_luaState);
}

void LuaSaveParser::printError(lua_State *luaState) {
  printf("%s\n", lua_tostring(luaState, -1));
}

void LuaSaveParser::luaInit(std::string filetype) {
  m_luaState = luaL_newstate();

  luaL_openlibs(m_luaState);

  *static_cast<LuaSaveParser**>(lua_getextraspace(m_luaState)) = this;

  const luaL_Reg regs[] {
    { "getSaveFileBuffer", &dispatch<&LuaSaveParser::lua_getSaveFileBuffer> },
    { "getSaveFileString", &dispatch<&LuaSaveParser::lua_getSaveFileString> },
    { "getStrArgs", &dispatch<&LuaSaveParser::lua_getStrArgs> },
    { "getIntArgs", &dispatch<&LuaSaveParser::lua_getIntArgs> },
    { nullptr, nullptr }
  };

  luaL_newlib(m_luaState, regs);
  lua_setglobal(m_luaState, "edizon");

  std::string path = "/EdiZon/editor/scripts/";
  path += filetype;
  path += ".lua";

  luaL_loadfile(m_luaState, path.c_str());

  if(lua_pcall(m_luaState, 0, 0, 0))
    printError(m_luaState);

  printf("Lua interpreter initialized!\n");
}

void LuaSaveParser::luaDeinit() {
  lua_close(m_luaState);
}

void LuaSaveParser::setLuaSaveFileBuffer(u8 *buffer, size_t bufferSize) {
  this->m_buffer = buffer;
  this->m_bufferSize = bufferSize;
}

void LuaSaveParser::setLuaArgs(std::vector<s32> intArgs, std::vector<std::string> strArgs) {
  this->m_intArgs = intArgs;
  this->m_strArgs = strArgs;
}

s32 LuaSaveParser::getValueFromSaveFile() {
  s32 out;

  lua_getglobal(m_luaState, "getValueFromSaveFile");
  if(lua_pcall(m_luaState, 0, 1, 0))
    printError(m_luaState);

  out = lua_tointeger(m_luaState, -1);
  lua_pop(m_luaState, 1);

  return out;
}

std::string LuaSaveParser::getStringFromSaveFile() {
  std::string out;

  lua_getglobal(m_luaState, "getStringFromSaveFile");
  if (lua_pcall(m_luaState, 0, 1, 0))
    printError(m_luaState);

  out = lua_tostring(m_luaState, -1);
  lua_pop(m_luaState, 1);

  return out;
}

void LuaSaveParser::setValueInSaveFile(s32 value) {
  lua_getglobal(m_luaState, "setValueInSaveFile");
  lua_pushinteger(m_luaState, value);
  if (lua_pcall(m_luaState, 1, 0, 0))
    printError(m_luaState);
}

void LuaSaveParser::setStringInSaveFile(std::string value) {
  lua_getglobal(m_luaState, "setStringInSaveFile");
  lua_pushstring(m_luaState, value.c_str());
  if (lua_pcall(m_luaState, 1, 0, 0))
    printError(m_luaState);
}

void LuaSaveParser::getModifiedSaveFile(std::vector<u8> &buffer, size_t *outSize) {
  lua_getglobal(m_luaState, "getModifiedSaveFile");
  if (lua_pcall(m_luaState, 0, 1, 0))
    printError(m_luaState);

  lua_pushnil(m_luaState);

	while (lua_next(m_luaState, 1)) {
		buffer.push_back(lua_tointeger(m_luaState, -1));
		lua_pop(m_luaState, 1);
    (*outSize)++;
	}

  lua_pop(m_luaState, 1);
}

int LuaSaveParser::lua_getSaveFileBuffer(lua_State *state) {
  lua_newtable(state);

  for (u32 i = 0; i < m_bufferSize; i++) {
    lua_pushnumber(state, i + 1);
    lua_pushnumber(state, m_buffer[i]);
    lua_rawset(state, -3);
  }

  lua_pushliteral(state, "n");
  lua_pushinteger(state, m_bufferSize);
  lua_rawset(state, -3);

  return 1;
}

int LuaSaveParser::lua_getSaveFileString(lua_State *state) {
  std::string str = reinterpret_cast<char*>(m_buffer);

  lua_pushstring(state, str.c_str());

  return 1;
}

int LuaSaveParser::lua_getStrArgs(lua_State *state) {
  lua_newtable(state);

  u16 index = 1;
  for (auto arg : m_strArgs) {
    lua_pushnumber(state, index++);
    lua_pushstring(state, arg.c_str());
    lua_rawset(state, -3);
  }

  lua_pushliteral(state, "n");
  lua_pushnumber(state, m_strArgs.size());
  lua_rawset(state, -3);

  return 1;
}

int LuaSaveParser::lua_getIntArgs(lua_State *state) {
  lua_newtable(state);

  u16 index = 1;
  for (auto arg : m_intArgs) {
    lua_pushnumber(state, index++);
    lua_pushnumber(state, arg);
    lua_rawset(state, -3);
  }

  lua_pushliteral(state, "n");
  lua_pushnumber(state, m_intArgs.size());
  lua_rawset(state, -3);

  return 1;
}
