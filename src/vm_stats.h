#pragma once

#include <cstdint>

struct vm_stats {
  uint64_t total_mem_access {0};
  uint64_t total_page_access {0};
  uint64_t num_page_fault {0};
  uint64_t num_swap_out {0};
};
