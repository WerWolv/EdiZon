#include "scripts.hpp"

static FILE *logFile;
static duk_ret_t print(duk_context *ctx);

Scripts::Scripts() {
  m_context = duk_create_heap_default();
  logFile = fopen("/EdiZon/cheats/sysmodule.log", "a+");
  
  duk_module_duktape_init(m_context);

  duk_push_global_object(m_context);
  duk_push_c_function(m_context, print, DUK_VARARGS);
  duk_put_prop_string(m_context, -2, "print");
  duk_pop(m_context);
}

Scripts::~Scripts() {
  duk_destroy_heap(m_context);
  fclose(logFile);
}


void Scripts::executeScripts() {

}

static duk_ret_t print(duk_context *ctx) {
  if (logFile == nullptr) return 1;

  fprintf(logFile, "Print: ");

  for (duk_idx_t i = 0, n = duk_get_top(ctx); i < n; i++) {
    if (i > 0)
      fprintf(logFile, " ");

    fprintf(logFile, "%s", duk_safe_to_string(ctx, i));
  }
  fprintf(logFile, "\n");
  fflush(logFile);

  return 0;
}