#include <getopt.h>
#include <iostream>
#include <memory>
#include <string>

#include "iceburg_simulator.h"
#include "universal_hashing_simulator.h"
#include "vm_simulator.h"
#include "vm_stats.h"

static void print_err_usage(const std::string& hint);
static void print_statistics(vm_stats& stats);

int main(int argc, char *argv[]) {

  FILE *trace = nullptr;
  char sim_option = '0';
  int opt;

  int mem_size_gb = 16;
  int bank_count = 128;

  // t: path to the trace file
  // s: simulator type, i for iceberg or u for universal
  // m: memory size in GB
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
        mem_size_gb = std::atoi(optarg);
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

  if (sim_option != 'i' && sim_option != 'u') {
    print_err_usage("Invalid simulator option");
  }

  std::unique_ptr<VmSimulator> simulator;

  if (sim_option == 'i') {
    simulator = std::make_unique<IceburgSimulator>();
  }
  else {
    simulator = std::make_unique<UniversalHashingSimulator>();
  }

  /* Begin reading the file */
  char rw;
  uint64_t address;

  while (!feof(trace)) {
    int ret = fscanf(trace, "%c 0x%llu\n", &rw, &address);
    if (ret != 2)
      continue;

    simulator->access(address, rw);
  }
}

static void print_err_usage(const std::string& hint) {
  std::cout << hint << '\n';
  std::cout << "usage:\n";
  std::cout << "./tlbsim -t <path-to-trace-file> -s <simulator, i or u >\n";
  exit(EXIT_FAILURE);
}

static void print_statistics(vm_stats& stats) {
  printf("Virtual Memory Statistics\n");
  printf("----------------\n");
  printf("\n");
}