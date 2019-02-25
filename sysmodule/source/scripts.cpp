#include "scripts.hpp"
#include <sstream>

static FILE *logFile;

static duk_ret_t print(duk_context *ctx);

typedef duk_ret_t (Scripts::*mem_func)(duk_context *);

template <mem_func func>
duk_ret_t dispatch(duk_context *ctx) {
  Scripts *ptr = reinterpret_cast<Scripts*>(duk_get_extra_space(ctx));
  return ((*ptr).*func)(ctx);
}

Scripts::Scripts(u64 baseAddr, u64 mainAddr, u64 heapAddr) : m_baseAddr(baseAddr), m_mainAddr(mainAddr), m_heapAddr(heapAddr) {
  m_context = duk_create_heap_default();
  duk_set_extra_space(m_context, this);

  logFile = fopen("/EdiZon/cheats/sysmodule.log", "a+");
  
  duk_module_duktape_init(m_context);

  duk_push_global_object(m_context);
  duk_push_c_function(m_context, print, DUK_VARARGS);
  duk_put_prop_string(m_context, -2, "print");

  duk_push_global_object(m_context);
  duk_push_c_function(m_context, &dispatch<&Scripts::readNBytes>, 2);
  duk_put_prop_string(m_context, -2, "readNBytes");

  duk_push_global_object(m_context);
  duk_push_c_function(m_context, &dispatch<&Scripts::writeNBytes>, 2);
  duk_put_prop_string(m_context, -2, "writeNBytes");
}

Scripts::~Scripts() {
  duk_destroy_heap(m_context);
  fclose(logFile);
}

void Scripts::setDebugger(Debugger *debugger) {
  m_debugger = debugger;
}

void Scripts::initScripts(std::string code) {
  std::stringstream ss;
  duk_eval_string(m_context, code.c_str());

  ss << "Addresses = { BASE : u64(0x" << std::hex << (m_baseAddr >> 32) << ", 0x" << (m_baseAddr & 0xFFFFFFFF) << "), ";
  ss << "MAIN : u64(0x" << std::hex << (m_mainAddr >> 32) << ", 0x" << (m_mainAddr & 0xFFFFFFFF) << "), ";
  ss << "HEAP : u64(0x" << std::hex << (m_heapAddr >> 32) << ", 0x" << (m_heapAddr & 0xFFFFFFFF) << ") };";

  duk_eval_string(m_context, ss.str().c_str());
}

void Scripts::finalizeScripts() {
  
}

void Scripts::executeScripts(std::string cheatName) {
  duk_get_global_string(m_context, cheatName.c_str());
  duk_call(m_context, 0);
  duk_pop(m_context);
}

static duk_ret_t print(duk_context *ctx) {
  if (logFile == nullptr) return 1;

  fprintf(logFile, "INFO: ");

  for (duk_idx_t i = 0, n = duk_get_top(ctx); i < n; i++) {
    if (i > 0)
      fprintf(logFile, " ");

    fprintf(logFile, "%s", duk_safe_to_string(ctx, i));
  }
  fprintf(logFile, "\n");
  fflush(logFile);

  return 0;
}

// readNBytes(addr, nBytes) -> bytes
duk_ret_t Scripts::readNBytes(duk_context *ctx) {
  m_debugger->breakProcess();

  u8 *addrBuffer;
  u8 *dataBuffer;
  size_t addrBufferSize;
  size_t dataBufferSize;
  u64 addr = 0;

  addrBuffer = (u8*)duk_get_buffer_data(ctx, -2, &addrBufferSize);
  dataBufferSize = duk_get_number(ctx, -1);

  dataBuffer = new u8[dataBufferSize];

  for (u8 i = 0; i < addrBufferSize; i++)
    addr |= addrBuffer[i] << i * 8;

  m_debugger->readMemory(dataBuffer, dataBufferSize, addr);

  m_debugger->continueProcess();

  u8 *retBuffer = (u8*)duk_push_fixed_buffer(ctx, dataBufferSize);
  memcpy(retBuffer, dataBuffer, dataBufferSize);

  fprintf(logFile, "Read from %lx [%lu]\n", addr, addrBufferSize);

  return 1;
}

// readNBytes(addr, bytes) -> void
duk_ret_t Scripts::writeNBytes(duk_context *ctx) {
  m_debugger->breakProcess();

  u8 *addrBuffer;
  u8 *dataBuffer;
  size_t addrBufferSize;
  size_t dataBufferSize;
  u64 addr = 0;

  addrBuffer = (u8*)duk_get_buffer_data(ctx, -2, &addrBufferSize);
  dataBuffer = (u8*)duk_get_buffer_data(ctx, -1, &dataBufferSize);

  for (u8 i = 0; i < addrBufferSize; i++)
    addr |= addrBuffer[i] << i * 8;

  m_debugger->writeMemory(dataBuffer, dataBufferSize, addr);

  fprintf(logFile, "Write to %lx [%lu]\n", addr, addrBufferSize);

  m_debugger->continueProcess();

  return 0;
}
