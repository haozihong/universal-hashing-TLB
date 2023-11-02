#pragma once

#include "vm_stats.h"

class VmSimulator {
public:
  virtual void access(uint64_t addr, char rw) = 0;

  inline vm_stats get_stats() const { return stats; }

protected:
  vm_stats stats;
};
