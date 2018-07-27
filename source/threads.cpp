#include "threads.hpp"

#include <vector>

static std::vector<Thread> threads;

Result Threads::create(ThreadFunc func) {
  Thread thread;

  Result res;
  if (R_FAILED( res = threadCreate(&thread, func, nullptr, 0x2000, 0x2B, -2)))
    return res;

  if (R_FAILED( res = threadStart(&thread)))
    return res;

  threads.push_back(thread);

  return 0;
}

void Threads::joinAll() {
  for (auto &thread : threads) {
    threadWaitForExit(&thread);
    threadClose(&thread);
  }
}
