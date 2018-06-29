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

LuaSaveParser::LuaSaveParser(std::string filetype) : m_filetype(filetype) {

}

LuaSaveParser::~LuaSaveParser() {
  lua_close(m_luaState);
}

void LuaSaveParser::luaInit() {
  m_luaState = luaL_newstate();

  luaL_openlibs(m_luaState);

  *static_cast<LuaSaveParser**>(lua_getextraspace(m_luaState)) = this;

  const luaL_Reg regs[] {
    { "getSaveFileBuffer", &dispatch<&LuaSaveParser::lua_getSaveFileBuffer> },
    { "getStrArgs", &dispatch<&LuaSaveParser::lua_getStrArgs> },
    { "getIntArgs", &dispatch<&LuaSaveParser::lua_getIntArgs> },
    { nullptr, nullptr }
  };

  luaL_newlib(m_luaState, regs);
  lua_setglobal(m_luaState, "edizon");

  luaL_loadfile(m_luaState, "/EdiZon/editor/scripts/bin.lua");

  lua_call(m_luaState, 0, 0);

  printf("Lua interpreter initialized!\n");
}

void LuaSaveParser::setLuaSaveFileBuffer(u8 *buffer, size_t bufferSize) {
  this->m_buffer = buffer;
  this->m_bufferSize = bufferSize;
}

void LuaSaveParser::setLuaArgs(std::vector<u64> intArgs, std::vector<std::string> strArgs) {
  this->m_intArgs = intArgs;
  this->m_strArgs = strArgs;
}

u64 LuaSaveParser::getValueFromSaveFile() {
  u64 out;

  lua_getglobal(m_luaState, "getValueFromSaveFile");
  lua_pcall(m_luaState, 0, 1, 0);

  out = lua_tointeger(m_luaState, -1);
  lua_pop(m_luaState, 1);

  return out;
}

std::string LuaSaveParser::getStringFromSaveFile() {
  std::string out;

  lua_getglobal(m_luaState, "getStringFromSaveFile");
  lua_pcall(m_luaState, 0, 1, 0);

  out = lua_tostring(m_luaState, -1);
  lua_pop(m_luaState, 1);

  return out;
}

void LuaSaveParser::setValueInSaveFile(u64 value) {
  lua_getglobal(m_luaState, "setValueInSaveFile");
  lua_pushinteger(m_luaState, value);
  lua_pcall(m_luaState, 1, 0, 0);
}

void LuaSaveParser::setStringInSaveFile(std::string value) {
  lua_getglobal(m_luaState, "setStringInSaveFile");
  lua_pushstring(m_luaState, value.c_str());
  lua_pcall(m_luaState, 1, 0, 0);
}

std::vector<u8> LuaSaveParser::getModifiedSaveFile() {
  std::vector<u8> buffer;

  lua_getglobal(m_luaState, "getModifiedSaveFile");
  lua_pcall(m_luaState, 0, 1, 0);

  lua_pushnil(m_luaState);

	while (lua_next(m_luaState, 1)) {
		buffer.push_back(lua_tointeger(m_luaState, -1));
		lua_pop(m_luaState, 1);
	}

  lua_pop(m_luaState, 1);

  return buffer;
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
