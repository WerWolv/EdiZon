#pragma once

#include <vector>
#include <string>

extern "C" {
  #include "duktape.h"
  #include "duk_module_duktape.h"
}

#include "debugger.hpp"

class Scripts {
public:
  Scripts();
  ~Scripts();

  void setDebugger(Debugger *debugger);
  void initScripts(std::string code);
  void finalizeScripts();
  void executeScripts(std::string cheatName);

private:
  Debugger *m_debugger;

  std::vector<std::string> m_loadedCheats;
  duk_context *m_context;

  duk_ret_t readNBytes(duk_context *ctx);
  duk_ret_t writeNBytes(duk_context *ctx);

};