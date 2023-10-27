#pragma once

#include <cstdint>

struct vm_stats {
  uint64_t total_instructions;            // Use the inst num field of the final branch
  uint64_t num_branch_instructions;
  uint64_t num_branches_correctly_predicted;
  uint64_t num_branches_mispredicted;
  uint64_t misses_per_kilo_instructions;  // Yes this is indeed being recorded as a uint64_t
                                          // Allows tracking various phases of program execution
                                          // and is a common metric in branch prediction papers

  double fraction_branch_instructions;    // fraction of the entire program that is branches
  double prediction_accuracy;
};
