#pragma once

#include <stdint.h>

struct PageFrame {
  uint64_t vpn;
  uint64_t timestamp {0};
  bool free {true};

  PageFrame() = default;
  PageFrame(uint64_t vpn, uint64_t ts, bool free): vpn(vpn), timestamp(ts), free(free) {}
};