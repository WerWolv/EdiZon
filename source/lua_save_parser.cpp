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


double add(lua_State *s, double a, double b) {
  double z;

  lua_getglobal(s, "add");  /* function to be called */
  lua_pushnumber(s, a);   /* push 1st argument */
  lua_pushnumber(s, b);   /* push 2nd argument */

              /* do the call (2 arguments, 1 result) */
  lua_pcall(s, 2, 1, 0);

  z = lua_tonumber(s, -1);
  lua_pop(s, 1);  /* pop returned value */
  return z;
}


LuaSaveParser::LuaSaveParser(std::string filetype, u8 *buffer, size_t bufferSize) : m_filetype(filetype), m_buffer(buffer), m_bufferSize(bufferSize) {
  this->setLuaArguments({ 5, 10, 15, 20, 25 }, { "Hello", "World", "Foo", "Bar", "SeeS" });
  m_luaState = luaL_newstate();

  luaL_openlibs(m_luaState);

  *static_cast<LuaSaveParser**>(lua_getextraspace(m_luaState)) = this;

  const luaL_Reg regs[] {
    { "getSaveFileBuffer", &dispatch<&LuaSaveParser::lua_getSaveFileBuffer>},
    { "getStrArguments", &dispatch<&LuaSaveParser::lua_getStrArguments>},
    { "getIntArguments", &dispatch<&LuaSaveParser::lua_getIntArguments>},
    { nullptr, nullptr }
  };

  luaL_newlib(m_luaState, regs);
  lua_setglobal(m_luaState, "edizon");

  luaL_loadfile(m_luaState, "/EdiZon/editor/scripts/bin.lua");

  lua_call(m_luaState, 0, 0);

  printf("Lua interpreter initialized!\n");

  u64 i;
  std::string str;

  getValueFromSaveFile(111, i);
  getValueFromSaveFile("Hello", str);

  printf("%lu %s\n", i, str.c_str());

}

LuaSaveParser::~LuaSaveParser() {
  lua_close(m_luaState);
}

void LuaSaveParser::setLuaArguments(std::vector<u64> intArguments, std::vector<std::string> strArguments) {
  this->m_intArguments = intArguments;
  this->m_strArguments = strArguments;
}

void LuaSaveParser::getValueFromSaveFile(u64 location, u64 &out) {
  lua_getglobal(m_luaState, "getValueFromSaveFile");
  std::cout << "IsFunction: " << lua_isfunction(m_luaState, -1) << std::endl;
  lua_pushinteger(m_luaState, (u32)location);
  lua_pcall(m_luaState, 1, 1, 0);

  out = lua_tointeger(m_luaState, -1);
  lua_pop(m_luaState, 1);
}

void LuaSaveParser::getValueFromSaveFile(std::string location, u64 &out) {
  lua_getglobal(m_luaState, "getValueFromSaveFile");
  lua_pushstring(m_luaState, location.c_str());
  lua_pcall(m_luaState, 1, 1, 0);

  out = lua_tointeger(m_luaState, -1);
  lua_pop(m_luaState, 1);
}

void LuaSaveParser::getValueFromSaveFile(u64 location, std::string &out) {
  lua_getglobal(m_luaState, "getValueFromSaveFile");
  lua_pushinteger(m_luaState, location);
  lua_pcall(m_luaState, 1, 1, 0);

  out = lua_tostring(m_luaState, -1);
  lua_pop(m_luaState, 1);
}

void LuaSaveParser::getValueFromSaveFile(std::string location, std::string &out) {
  lua_getglobal(m_luaState, "getValueFromSaveFile");
  lua_pushstring(m_luaState, location.c_str());
  lua_pcall(m_luaState, 1, 1, 0);

  out = lua_tostring(m_luaState, -1);
  lua_pop(m_luaState, 1);
}

void LuaSaveParser::setValueInSaveFile(u64 location, u64 value) {
  lua_getglobal(m_luaState, "getValueFromSaveFile");
  lua_pushinteger(m_luaState, location);
  lua_pushinteger(m_luaState, value);
  lua_pcall(m_luaState, 2, 0, 0);

  lua_pop(m_luaState, 2);
}

void LuaSaveParser::setValueInSaveFile(std::string location, u64 value) {
  lua_getglobal(m_luaState, "getValueFromSaveFile");
  lua_pushstring(m_luaState, location.c_str());
  lua_pushinteger(m_luaState, value);
  lua_pcall(m_luaState, 2, 0, 0);

  lua_pop(m_luaState, 2);
}

void LuaSaveParser::setValueInSaveFile(u64 location, std::string value) {
  lua_getglobal(m_luaState, "getValueFromSaveFile");
  lua_pushinteger(m_luaState, location);
  lua_pushstring(m_luaState, value.c_str());
  lua_pcall(m_luaState, 2, 0, 0);

  lua_pop(m_luaState, 2);
}

void LuaSaveParser::setValueInSaveFile(std::string location, std::string value) {
  lua_getglobal(m_luaState, "getValueFromSaveFile");
  lua_pushstring(m_luaState, location.c_str());
  lua_pushstring(m_luaState, value.c_str());
  lua_pcall(m_luaState, 2, 0, 0);

  lua_pop(m_luaState, 2);
}

int LuaSaveParser::lua_getSaveFileBuffer(lua_State *state) {
  lua_newtable(state);

  for (u32 i = 0; i < m_bufferSize; i++) {
    lua_pushnumber(state, i);
    lua_pushinteger(state, m_buffer[i]);
    lua_rawset(state, -3);
  }

  lua_pushliteral(state, "n");
  lua_pushinteger(state, m_bufferSize);
  lua_rawset(state, -3);

  return 1;
}

int LuaSaveParser::lua_getStrArguments(lua_State *state) {
  lua_newtable(state);

  u16 index = 0;
  for (auto argument : m_strArguments) {
    lua_pushnumber(state, index++);
    lua_pushstring(state, argument.c_str());
    lua_rawset(state, -3);
  }

  lua_pushliteral(state, "n");
  lua_pushnumber(state, index);
  lua_rawset(state, -3);

  return 1;
}

int LuaSaveParser::lua_getIntArguments(lua_State *state) {
  lua_newtable(state);

  u16 index = 0;
  for (auto argument : m_intArguments) {
    lua_pushinteger(state, index++);
    lua_pushinteger(state, argument);
    lua_rawset(state, -3);
  }

  lua_pushliteral(state, "n");
  lua_pushnumber(state, index);
  lua_rawset(state, -3);

  return 1;
}
