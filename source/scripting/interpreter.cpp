#include "scripting/interpreter.hpp"

#include <iostream>
#include <algorithm>

#include "helpers/encoding.hpp"

#define LLONG_MAX INT64_MAX
#define LLONG_MIN INT64_MIN
#include "lua.hpp"

Interpreter::Interpreter() {

}

Interpreter::~Interpreter() {
    
}

void Interpreter::setSaveFileBuffer(u8 *buffer, size_t bufferSize, std::string encoding) {
  std::vector<u8> utf8;

  std::transform(encoding.begin(), encoding.end(), encoding.begin(), ::tolower);

  if (encoding == "ascii")
    m_encoding = ASCII;
  else if (encoding == "utf-8")
    m_encoding = UTF_8;
  else if (encoding == "utf-16le")
    m_encoding = UTF_16LE;
  else if (encoding == "utf-16be")
    m_encoding = UTF_16BE;
  else printf("Lua init warning: Invalid encoding, using ASCII\n");

  switch (m_encoding) {
    case UTF_16BE:
      utf8 = Encoding::uft16beToUtf8(buffer, bufferSize);
      break;
    case UTF_16LE:
      utf8 = Encoding::uft16leToUtf8(buffer, bufferSize);
      break;
    case ASCII: [[fallthrough]]
    case UTF_8: [[fallthrough]]
    default:
      utf8 = std::vector<u8>(buffer, buffer + bufferSize);
      break;
  }

  m_buffer.clear();

  m_bufferSize = utf8.size();
  m_buffer.reserve(m_bufferSize);

  for (u32 i = 0; i < this->m_bufferSize; i++)
    m_buffer[i] = utf8[i];
}

void Interpreter::setArgs(std::vector<s32> intArgs, std::vector<std::string> strArgs) {
  m_intArgs = intArgs;
  m_strArgs = strArgs;
}

double Interpreter::evaluateEquation(std::string equation, s64 value) {
  lua_State *s = luaL_newstate();
  double ret;
  std::string func = "function eval(value)\n";
  func += "return ";
  func += equation;
  func += "\nend";

  luaL_dostring(s, func.c_str());
  lua_getglobal(s, "eval");
  lua_pushinteger(s, value);
  if (lua_pcall(s, 1, 1, 0))
      printf("%s\n", lua_tostring(s, -1));

  ret = lua_tonumber(s, -1);

  if (s != nullptr) {
    lua_close(s);
    s = nullptr;
  }

  return ret;
}