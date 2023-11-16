#include "pin.H"
#include <memory>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <unordered_set>

#include "../iceberg_simulator.h"
#include "../universal_hashing_simulator.h"
#include "../conventional_vm_simulator.h"
#include "../vm_simulator.h"
#include "../vm_stats.h"

using std::cerr, std::cout;
using std::endl;
using std::string;
using std::unique_ptr;
using std::make_unique;

FILE *output = nullptr;

static unique_ptr<VmSimulator> simulator;

static std::unordered_set<string> sim_options {
  "ice", "con", "uni-static", "uni-dyn", "uni-dyn-ind"
};

 static uint64_t access_cnt = 0;

/* ===================================================================== */
// Command line switches
/* ===================================================================== */

KNOB<string> KnobOutputFileName(KNOB_MODE_WRITEONCE, "pintool", "o", "pin-sim-log.txt",
                      "output file name");

KNOB<string> KnobSimulatorSel(KNOB_MODE_WRITEONCE, "pintool", "s", "",
                      "which simulator to use (ice, con, uni-static, uni-dyn, uni-dyn-ind)");

KNOB<double> KnobMemSizeMB(KNOB_MODE_WRITEONCE, "pintool", "m", "128.0",
                      "memory size in mb, can be a decimal");

KNOB<int> KnobWayCount(KNOB_MODE_WRITEONCE, "pintool", "w", "32",
                      "for universal hashing: number of ways(banks)");

KNOB<int> KnobFrontyardSize(KNOB_MODE_WRITEONCE, "pintool", "f", "56",
                      "for iceberg hashing: frontyard size");

KNOB<int> KnobBackyardSize(KNOB_MODE_WRITEONCE, "pintool", "b", "8",
                      "for iceberg hashing: backyard size");

/* ===================================================================== */
// Utilities
/* ===================================================================== */

/*!
 *  Print out help message.
 */
INT32 Usage() {
  cerr << "This Pintool prints the IPs of every instruction executed and the trace of memory "
          "addresses\n";
  cerr << KNOB_BASE::StringKnobSummary() << endl;

  return -1;
}

/* ===================================================================== */
// Instrumentation callbacks
/* ===================================================================== */

inline void access(VOID *addr, char rw) {
  simulator->access((uint64_t)addr, rw);

  access_cnt += 1;
  if (access_cnt % 100000000 == 0) {
    simulator->get_stats().fprint(output);
  }
}

// Print a instruction
VOID RecordInst(VOID *addr) {
  access(addr, 'r');
}

// Print a memory read record
VOID RecordMemRead(VOID *ip, VOID *addr) {
  access(addr, 'r');
}

// Print a memory write record
VOID RecordMemWrite(VOID *ip, VOID *addr) {
  access(addr, 'w');
}

// Is called for every instruction and instruments reads and writes
VOID Instruction(INS ins, VOID *v) {
  // Insert a call to printip before every instruction, and pass it the IP
  INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)RecordInst, IARG_INST_PTR, IARG_END);

  // Instruments memory accesses using a predicated call, i.e.
  // the instrumentation is called iff the instruction will actually be executed.
  //
  // On the IA-32 and Intel(R) 64 architectures conditional moves and REP
  // prefixed instructions appear as predicated instructions in Pin.
  UINT32 memOperands = INS_MemoryOperandCount(ins);

  // Iterate over each memory operand of the instruction.
  for (UINT32 memOp = 0; memOp < memOperands; memOp++) {
    if (INS_MemoryOperandIsRead(ins, memOp)) {
      INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)RecordMemRead, IARG_INST_PTR,
                               IARG_MEMORYOP_EA, memOp, IARG_END);
    }
    // Note that in some architectures a single memory operand can be
    // both read and written (for instance incl (%eax) on IA-32)
    // In that case we instrument it once for read and once for write.
    if (INS_MemoryOperandIsWritten(ins, memOp)) {
      INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)RecordMemWrite, IARG_INST_PTR,
                               IARG_MEMORYOP_EA, memOp, IARG_END);
    }
  }
}

/*!
 * Print out analysis results.
 * This function is called when the application exits.
 * @param[in]   code            exit code of the application
 * @param[in]   v               value specified by the tool in the
 *                              PIN_AddFiniFunction function call
 */
VOID Fini(INT32 code, VOID *v) {
  simulator->get_stats().fprint(output);
}

/*!
 * The main procedure of the tool.
 * This function is called when the application image is loaded but not yet started.
 */
int main(int argc, char *argv[]) {
  // Initialize PIN library. Print help message if -h(elp) is specified
  // in the command line or the command line is invalid
  if (PIN_Init(argc, argv))
    return Usage();

  output = fopen(KnobOutputFileName.Value().c_str(), "w");
  if (output == nullptr) {
    fprintf(stderr, "cannot open the output file.\n");
    exit(EXIT_FAILURE);
  }

  string sim_option = KnobSimulatorSel.Value();
  double mem_size_mb = KnobMemSizeMB.Value();
  int way_count = KnobWayCount.Value();
  int fyard_size = KnobFrontyardSize.Value();
  int byard_size = KnobBackyardSize.Value();

  if (sim_option == "ice") {
    simulator = make_unique<IcebergSimulator>(mem_size_mb, fyard_size, byard_size);
  }
  else if (sim_option == "con") {
    simulator = make_unique<ConventionalVmSimulator>(mem_size_mb);
  }
  else if (sim_options.count(sim_option) == 1) {
    simulator = make_unique<UniversalHashingSimulator>(mem_size_mb, way_count, sim_option);
  }
  else {
    fprintf(stderr, "unknown simulator option.\n");
    exit(EXIT_FAILURE);
  }

  INS_AddInstrumentFunction(Instruction, 0);
  PIN_AddFiniFunction(Fini, 0);

  // Start the program, never returns
  PIN_StartProgram();

  return 0;
}
