#include "scripting/python_interpreter.hpp"

#include <iostream>
#include <algorithm>
#include <cstring>

#include "helpers/encoding.hpp"

static void *python_extraspace;
static PyMethodDef *edizonMethods;
static PyModuleDef edizonModule;

PythonInterpreter::PythonInterpreter() {
    m_mainObject = nullptr;
    m_buffer.clear();
}

PythonInterpreter::~PythonInterpreter() {
    if (m_mainObject != nullptr) {
        Py_Finalize();
        m_mainObject = nullptr;
    }
}

typedef PyObject *(PythonInterpreter::*mem_func)(PyObject *, PyObject *);

template <mem_func func>
PyObject *dispatch(PyObject *self, PyObject *args) {
  PythonInterpreter *ptr = *reinterpret_cast<PythonInterpreter**>(&python_extraspace);
  return ((*ptr).*func)(self, args);
}

PyMODINIT_FUNC PyInit_edizon() {
  PyObject *m;

  m = PyModule_Create(&edizonModule);

  if (m == NULL)
    return NULL;

  PyObject *error = PyErr_NewException("edizon.error", NULL, NULL);

  Py_INCREF(error);

  PyModule_AddObject(m, "error", error);
  return m;
}

bool PythonInterpreter::initialize(std::string filetype) {
  python_extraspace = this;
  m_buffer.clear();

  edizonMethods = new PyMethodDef[5];
  edizonMethods[0] = { "getSaveFileBuffer", &dispatch<&PythonInterpreter::py_getSaveFileBuffer>, METH_NOARGS, "getSaveFileBuffer" };
  edizonMethods[1] = { "getSaveFileString", &dispatch<&PythonInterpreter::py_getSaveFileString>, METH_NOARGS, "getSaveFileString" };
  edizonMethods[2] = { "getStrArgs", &dispatch<&PythonInterpreter::py_getStrArgs>, METH_NOARGS, "getStrArgs" };
  edizonMethods[3] = { "getIntArgs", &dispatch<&PythonInterpreter::py_getIntArgs>, METH_NOARGS, "getIntArgs" };
  edizonMethods[4] = { NULL, NULL, 0, NULL };

  edizonModule = { PyModuleDef_HEAD_INIT, "edizon", "EdiZon functions", 0, edizonMethods, NULL, NULL, NULL, NULL };

  Py_NoSiteFlag = 1;
  Py_IgnoreEnvironmentFlag = 1;
  Py_NoUserSiteDirectory = 1;

  char cwd[256];
  getcwd(cwd, sizeof(cwd));

  char *stripped_cwd = strchr(cwd, '/');
  if (stripped_cwd == NULL) stripped_cwd = cwd;

  Py_SetPythonHome(Py_DecodeLocale(EDIZON_DIR "/editor/scripts/", NULL));

	PyImport_AppendInittab("edizon", PyInit_edizon);

  Py_Initialize();

	PyObject *moduleMainString = PyUnicode_FromString("__main__");
	m_mainObject = PyImport_Import(moduleMainString);

  std::string path = EDIZON_DIR "/editor/scripts/";
  path += filetype;
  path += ".py";

  FILE *fp = fopen(path.c_str(), "r");

  if (fp == nullptr) {
    printf("Failed to load file\n");
    return false;
  }

  if (PyRun_SimpleFileEx(fp, path.c_str(), true) != 0) {
    if (PyErr_Occurred() != nullptr) {
      PyErr_Print();
      printf("\n");
    }

      return false;
  }

  printf("Python interpreter initialized!\n");

  return true;
}

void PythonInterpreter::deinitialize() {
    if (m_mainObject != nullptr) {
        Py_Finalize();
        m_mainObject = nullptr;
    }
}

s64 PythonInterpreter::getValueFromSaveFile() {
	PyObject *func = PyObject_GetAttrString(m_mainObject, "getValueFromSaveFile");

  if (func == nullptr) {
    printf("Failed to call python function getValueFromSaveFile!\n");
    return 0;
  }

	PyObject *result = PyObject_CallObject(func, nullptr);

  if (PyErr_Occurred() != nullptr) {
    PyErr_Print();
    printf("\n");
  }

  if (result == nullptr) return 0;

  return PyLong_AsLong(result);
}

s64 PythonInterpreter::getDummyValue() {
	PyObject *func = PyObject_GetAttrString(m_mainObject, "getDummyValue");

  if (func == nullptr) {
    printf("Failed to call python function getDummyValue!\n");
    return 0;
  }
	PyObject *result = PyObject_CallObject(func, nullptr);

  if (PyErr_Occurred() != nullptr) {
    PyErr_Print();
    printf("\n");
  }

  if (result == nullptr) return 0;

  return PyLong_AsLong(result);
}


std::string PythonInterpreter::getStringFromSaveFile() {
	PyObject *func = PyObject_GetAttrString(m_mainObject, "getStringFromSaveFile");

  if (func == nullptr) {
    printf("Failed to call python function getStringFromSaveFile!\n");
    return "NULL";
  }
	PyObject *result = PyObject_CallObject(func, nullptr);

  if (PyErr_Occurred() != nullptr) {
    PyErr_Print();
    printf("\n");
  }

  if (result == nullptr) return "NULL";

  return std::string(PyBytes_AsString(PyUnicode_AsEncodedString(result, "utf-8", "~E~")));
}

std::string PythonInterpreter::getDummyString() {
	PyObject *func = PyObject_GetAttrString(m_mainObject, "getDummyString");

  if (func == nullptr) {
    printf("Failed to call python function getDummyString!\n");
    return "NULL";
  }
	PyObject *result = PyObject_CallObject(func, nullptr);

  if (PyErr_Occurred() != nullptr) {
    PyErr_Print();
    printf("\n");
  }

  if (result == nullptr) return "NULL";

  return std::string(PyBytes_AsString(PyUnicode_AsEncodedString(result, "utf-8", "~E~")));
}

void PythonInterpreter::setValueInSaveFile(s64 value) {
	PyObject *func = PyObject_GetAttrString(m_mainObject, "setValueInSaveFile");

  if (func == nullptr) {
    printf("Failed to call python function setValueInSaveFile!\n");
    return;
  }

	PyObject_CallObject(func, PyTuple_Pack(1, PyLong_FromLong(value)));

  if (PyErr_Occurred() != nullptr) {
    PyErr_Print();
    printf("\n");
  }
}

void PythonInterpreter::setStringInSaveFile(std::string value) {
	PyObject *func = PyObject_GetAttrString(m_mainObject, "setStringInSaveFile");

  if (func == nullptr) {
    printf("Failed to call python function setStringInSaveFile!\n");
    return;
  }

	PyObject_CallObject(func, PyTuple_Pack(1, PyUnicode_FromString(value.c_str())));

  if (PyErr_Occurred() != nullptr) {
    PyErr_Print();
    printf("\n");
  }
}

void PythonInterpreter::setDummyValue(s64 value) {
	PyObject *func = PyObject_GetAttrString(m_mainObject, "setDummyValue");

  if (func == nullptr) {
    printf("Failed to call python function setDummyValue!\n");
    return;
  }

	PyObject_CallObject(func, PyTuple_Pack(1, PyLong_FromLong(value)));

  if (PyErr_Occurred() != nullptr) {
    PyErr_Print();
    printf("\n");
  }
}

void PythonInterpreter::setDummyString(std::string value) {
	PyObject *func = PyObject_GetAttrString(m_mainObject, "setDummyString");

  if (func == nullptr) {
    printf("Failed to call python function setDummyString!\n");
    return;
  }

	PyObject_CallObject(func, PyTuple_Pack(1, PyUnicode_FromString(value.c_str())));

  if (PyErr_Occurred() != nullptr) {
    PyErr_Print();
    printf("\n");
  }
}

void PythonInterpreter::getModifiedSaveFile(std::vector<u8> &buffer) {
  std::vector<u8> encoded;

	PyObject *func = PyObject_GetAttrString(m_mainObject, "getModifiedSaveFile");

  if (func == nullptr) {
    printf("Failed to call python function getModifiedSaveFile!\n");
    return;
  }

	PyObject *result = PyObject_CallObject(func, nullptr);
  if (PyErr_Occurred() != nullptr) {
    PyErr_Print();
    printf("\n");
  }

  if (result == nullptr) return;

  size_t size = PyByteArray_Size(result);
  char *data = PyByteArray_AsString(result);

  if (PyErr_Occurred() != nullptr) {
    PyErr_Print();
    printf("\n");
  }

  encoded.reserve(size);
  
  for (u64 i = 0; i < size; i++)
    encoded.push_back(data[i]);

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

std::string PythonInterpreter::callFunction(std::string funcName) {
	PyObject *func = PyObject_GetAttrString(m_mainObject, funcName.c_str());

  if (func == nullptr) {
    printf("Failed to call python function %s!\n", funcName.c_str());
    return "";
  }

	PyObject *result = PyObject_CallObject(func, nullptr);

  if (PyErr_Occurred() != nullptr) {
    PyErr_Print();
    printf("\n");
  }

  if (result == nullptr) return "";

  return std::string(PyBytes_AsString(PyUnicode_AsEncodedString(result, "utf-8", "~E~")));
} 

PyObject *PythonInterpreter::py_getSaveFileBuffer(PyObject *self, PyObject *args) {
  return PyByteArray_FromStringAndSize((char*)(&m_buffer[0]), m_bufferSize);
}

PyObject *PythonInterpreter::py_getSaveFileString(PyObject *self, PyObject *args) {
  std::string string((char*)(&m_buffer[0]));

  return PyUnicode_FromStringAndSize(string.c_str(), string.length());
}

PyObject *PythonInterpreter::py_getStrArgs(PyObject *self, PyObject *args) {
  if (m_strArgs.empty()) return PyLong_FromLong(0);

  PyObject *strArgs = PyList_New(0);

  for (std::string arg : m_strArgs)
    PyList_Append(strArgs, PyUnicode_FromString(arg.c_str()));

  return strArgs;
}

PyObject *PythonInterpreter::py_getIntArgs(PyObject *self, PyObject *args) {
  if (m_intArgs.empty()) return PyLong_FromLong(0);

  PyObject *intArgs = PyList_New(0);

  for (s64 arg : m_intArgs)
    PyList_Append(intArgs, PyLong_FromLong(arg));

  return intArgs;
}