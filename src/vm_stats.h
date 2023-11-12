#pragma once

#include <cstdint>
#include <cstdio>

struct vm_stats {
  uint64_t total_mem_access {0};
  uint64_t total_page_access {0};
  uint64_t num_page_fault {0};
  uint64_t num_swap_out {0};

  void print() {
    printf("Virtual Memory Statistics\n");
    printf("----------------\n");
    printf("total memory access: %lu\n", total_mem_access);
    printf("total page access: %lu\n", total_page_access);
    printf("number of pagefaults: %lu\n", num_page_fault);
    printf("number of swap: %lu\n", num_swap_out);
    printf("\n");
  }
};
