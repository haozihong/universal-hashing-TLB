#pragma once

#include "vm_stats.h"

#include <iostream>
#include <unordered_set>

class VmSimulator {
public:
  virtual void access(uint64_t addr, char rw) = 0;

  virtual vm_stats get_stats() {
    stats.total_page_access = vpn_set.size();
    return stats;
  }

  virtual void print_info(std::ostream& os) {}

  virtual ~VmSimulator() {};

protected:
  vm_stats stats;
  std::unordered_set<uint64_t> vpn_set;
};
