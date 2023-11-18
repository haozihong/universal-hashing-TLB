#pragma once

#include <cstdint>
#include <cstdio>

struct vm_stats {
  uint64_t total_mem_access {0};
  uint64_t total_page_access {0};
  uint64_t num_page_fault {0};
  uint64_t num_swap_out {0};
  // measure the age of the block
  uint64_t total_age_of_swapped_out_pages {0};
  double avg_age_of_swapped_out_pages {0};
  // memory utilization when the first swap happens
  double mem_util_pct {0};

  void print() {
    fprint(stdout);
  }

  void fprint(FILE *file) {
    if (file == nullptr) return;

    fprintf(file, "Virtual Memory Statistics\n");
    fprintf(file, "----------------\n");
    fprintf(file, "total memory access: %lu\n", total_mem_access);
    fprintf(file, "total page access: %lu\n", total_page_access);
    fprintf(file, "number of pagefaults: %lu\n", num_page_fault);
    fprintf(file, "number of swap: %lu\n", num_swap_out);
    if (num_swap_out != 0) {
      fprintf(file, "first swap memory utilization: %lf\n", mem_util_pct);
      fprintf(file, "total age of swapped out pages: %lu\n", total_age_of_swapped_out_pages);
      fprintf(file, "average age of swapped out pages: %lu\n", total_age_of_swapped_out_pages / num_swap_out);
    }
    fprintf(file, "\n");
  }
};
