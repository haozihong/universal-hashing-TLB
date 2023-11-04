#include <getopt.h>
#include <iostream>
#include <memory>
#include <string>

#include "iceberg_simulator.h"
#include "universal_hashing_simulator.h"
#include "conventional_vm_simulator.h"
#include "vm_simulator.h"
#include "vm_stats.h"

static void print_err_usage(const std::string& hint);
static void print_statistics(const vm_stats& stats);

int main(int argc, char *argv[]) {

  FILE *trace = nullptr;
  char sim_option = '0';
  int opt;

  double mem_size_mb = 4096;
  int bank_count = 128;

  // t: path to the trace file
  // s: simulator type, i for iceberg or u for universal
  // m: memory size in mb, can be a decimal
  // b: for universal hashing: number of banks
  while (-1 != (opt = getopt(argc, argv, "t:s:m:b:"))) {
    switch (opt) {
      case 't':
        trace = std::fopen(optarg, "r");
        break;

      case 's':
        sim_option = optarg[0];
        break;

      case 'm':
        mem_size_mb = std::atof(optarg);
        break;

      case 'b':
        bank_count = std::atoi(optarg);
        break;

      default:
        print_err_usage("Invalid argument to program");
        break;
    }
  }

  if (trace == nullptr) {
    print_err_usage("Could not open the input trace file");
  }

  std::unique_ptr<VmSimulator> simulator;

  if (sim_option == 'i') {
    simulator = std::make_unique<IcebergSimulator>(mem_size_mb, 56, 8);
  }
  else if (sim_option == 'u') {
    simulator = std::make_unique<UniversalHashingSimulator>(mem_size_mb, bank_count);
  }
  else if (sim_option == 'c') {
    simulator = std::make_unique<ConventionalVmSimulator>(mem_size_mb);
  }
  else {
    print_err_usage("Invalid simulator option");
  }

  /* Begin reading the file */
  char rw;
  uint64_t address;

  while (!feof(trace)) {
    if (fscanf(trace, "%c 0x%llx\n", &rw, &address) != 2) {
      continue;
    }
    
    simulator->access(address, rw);
  }

  print_statistics(simulator->get_stats());
}

static void print_err_usage(const std::string& hint) {
  std::cout << hint << '\n';
  std::cout << "usage:\n";
  std::cout << "./tlbsim -t <path-to-trace-file> -s <simulator, i or u >\n";
  exit(EXIT_FAILURE);
}

static void print_statistics(const vm_stats& stats) {
  printf("Virtual Memory Statistics\n");
  printf("----------------\n");
  printf("\n");
  printf("total memory access: %llu\n", stats.total_mem_access);
  printf("total page access: %llu\n", stats.total_page_access);
  printf("number of pagefaults: %llu\n", stats.num_page_fault);
  printf("number of swap: %llu\n", stats.num_swap_out);
}