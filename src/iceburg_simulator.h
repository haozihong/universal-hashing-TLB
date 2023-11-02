#pragma once

#include "vm_simulator.h"

class IceburgSimulator : public VmSimulator {

  void access(uint64_t addr, char rw) override {}
};
