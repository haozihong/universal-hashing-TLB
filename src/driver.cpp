#include <getopt.h>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_set>

#include "iceberg_simulator.h"
#include "universal_hashing_simulator.h"
#include "conventional_vm_simulator.h"
#include "vm_simulator.h"
#include "vm_stats.h"

static void print_err_usage(const std::string& hint);

static std::unordered_set<std::string> sim_options {
  "ice", "con", "uni-static", "uni-dyn", "uni-dyn-ind", "uni-dyn-tbl", "uni-dyn-xor"
};

int main(int argc, char *argv[]) {

  FILE *trace = nullptr;
  std::string sim_option = "";
  int opt;

  double mem_size_mb = 4096;
  int way_count = 128;
  int fyard_size = 56;
  int byard_size = 8;

  // t: path to the trace file
  // s: simulator type, options are:
  //        ice: iceberg
  //        con: conventional
  //        uni-static: universal (static set-associative)
  //        uni-dyn: universal (dynamic set-associative)
  //        uni-dyn-ind: universal (dynamic independent set-associative)
  //        
  // m: memory size in mb, can be a decimal
  // w: for universal hashing: number of ways(banks)
  // f: for iceberg hashing: frontyard size
  // b: for iceberg hashing: backyard size
  while (-1 != (opt = getopt(argc, argv, "t:s:m:w:f:b:"))) {
    switch (opt) {
      case 't':
        trace = std::fopen(optarg, "rb");
        break;

      case 's':
        sim_option = std::string(optarg);
        break;

      case 'm':
        mem_size_mb = std::atof(optarg);
        break;

      case 'w':
        way_count = std::atoi(optarg);
        break;
      
      case 'f':
        fyard_size = std::atoi(optarg);
        break;

      case 'b':
        byard_size = std::atoi(optarg);
        break;

      default:
        print_err_usage("Invalid argument to program");
        break;
    }
  }

  std::unique_ptr<VmSimulator> simulator;

  if (sim_option == "ice") {
    simulator = std::make_unique<IcebergSimulator>(mem_size_mb, fyard_size, byard_size);
  }
  else if (sim_option == "con") {
    simulator = std::make_unique<ConventionalVmSimulator>(mem_size_mb);
  }
  else if (sim_options.count(sim_option) == 1) {
    simulator = std::make_unique<UniversalHashingSimulator>(mem_size_mb, way_count, sim_option);
  }
  else {
    print_err_usage("Invalid simulator option");
  }

  if (trace == nullptr) {
    if (feof(stdin)) {
      print_err_usage("Could not open the input trace file");
    }
    else {
      trace = fdopen(fileno(stdin), "rb");
    }
  }

  char rw;
  uint64_t address;
  uint64_t access_cnt = 0;

  /* Begin reading the file */
  while (!feof(trace)
          && fread(&rw, sizeof(char), 1, trace) == 1
          && fread(&address, sizeof(uint64_t), 1, trace) == 1) {
    
    simulator->access(address, rw);

    access_cnt += 1;
    if (access_cnt % 1000000 == 0) {
      simulator->get_stats().print();
    }
  }

  simulator->get_stats().print();
}

static void print_err_usage(const std::string& hint) {
  std::cout << hint << '\n';
  std::cout << "usage:\n";
  std::cout << "./tlbsim -t <path-to-trace-file> -s <simulator, i or u >\n";
  exit(EXIT_FAILURE);
}