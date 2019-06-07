#pragma once

#include <string>
#include <vector>

#include <edizon.h>

class Interpreter {
public:
  Interpreter();
  virtual ~Interpreter();

  virtual bool initialize(std::string filetype) = 0;
  virtual void deinitialize() = 0;

  virtual s64 getValueFromSaveFile() = 0;
  virtual std::string getStringFromSaveFile() = 0;

  virtual void setValueInSaveFile(s64 value) = 0;
  virtual void setStringInSaveFile(std::string value) = 0;

  virtual void getModifiedSaveFile(std::vector<u8> &buffer) = 0;

  virtual s64 getDummyValue() = 0;
  virtual std::string getDummyString() = 0;
  virtual void setDummyValue(s64 value) = 0;
  virtual void setDummyString(std::string value) = 0;

  virtual std::string callFunction(std::string funcName) = 0;

  void setArgs(std::vector<s32> intArgs, std::vector<std::string> strArgs);
  void setSaveFileBuffer(u8 *buffer, size_t bufferSize, std::string encoding);

  double evaluateEquation(std::string equation, s64 value);

protected:
  std::string m_filetype;
  std::vector<u8> m_buffer;
  u64 m_bufferSize;

  std::vector<s32> m_intArgs;
  std::vector<std::string> m_strArgs;

  enum {
    ASCII,
    UTF_8,
    UTF_16LE,
    UTF_16BE
  } m_encoding = ASCII;
};
