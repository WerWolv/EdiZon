#pragma once

#include <vector>
#include <string>

extern "C" {
  #include "duktape.h"
  #include "duk_module_duktape.h"
}

class Scripts {
public:
  Scripts();
  ~Scripts();

  void initScripts(std::string code);
  void finalizeScripts();
  void executeScripts(std::string cheatName);

private:
  std::vector<std::string> m_loadedCheats;
  duk_context *m_context;
};