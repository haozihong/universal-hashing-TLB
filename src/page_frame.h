#pragma once

#include <stdint.h>

struct PageFrame {
  uint64_t vpn;
  uint64_t timestamp {0};
  bool free {true};
};