#pragma once

#include "vm_stats.h"

class VmSimulator {
public:
  virtual void access(uint64_t addr, char rw) = 0;

  virtual vm_stats get_stats() { return stats; }

protected:
  vm_stats stats;
};
