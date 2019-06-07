#pragma once

#include "Python.h"

#include "interpreter.hpp"

#include <string>
#include <vector>

#include <edizon.h>

class PythonInterpreter : public Interpreter{
public:
  PythonInterpreter();
  ~PythonInterpreter();

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

  PyObject *py_getSaveFileBuffer(PyObject *self, PyObject *args);
  PyObject *py_getSaveFileString(PyObject *self, PyObject *args);
  PyObject *py_getStrArgs(PyObject *self, PyObject *args);
  PyObject *py_getIntArgs(PyObject *self, PyObject *args);


private:
  std::string m_filetype;

  PyObject *m_mainObject;
  PyObject m_globals, m_locals;

  enum {
    ASCII,
    UTF_8,
    UTF_16LE,
    UTF_16BE
  } m_encoding = ASCII;
};
