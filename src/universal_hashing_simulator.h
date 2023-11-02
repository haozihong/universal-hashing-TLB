#pragma once

#include "vm_simulator.h"

class UniversalHashingSimulator : public VmSimulator {
  void access(uint64_t addr, char rw) override {}
};
