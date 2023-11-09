#include "pin.H"
#include <fstream>
#include <iostream>
#include <stdio.h>
using std::cerr, std::cout;
using std::endl;
using std::string;

FILE *trace = NULL;

/* ===================================================================== */
// Command line switches
/* ===================================================================== */
KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "",
                            "specify file name for output");

KNOB<BOOL> KnobBinary(KNOB_MODE_WRITEONCE, "pintool", "binary", "0",
                      "whether output in binary form");

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

// Print a instruction
VOID RecordInst(VOID *addr) {
  fprintf(trace, "I %p\n", addr);
}

// Print a memory read record
VOID RecordMemRead(VOID *ip, VOID *addr) {
  fprintf(trace, "R %p\n", addr);
}

// Print a memory write record
VOID RecordMemWrite(VOID *ip, VOID *addr) {
  fprintf(trace, "W %p\n", addr);
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

// Print a instruction
VOID RecordInstBinary(VOID *addr) {
  putc('I', trace);
  fwrite(&addr, sizeof(VOID *), 1, trace);
}

// Print a memory read record
VOID RecordMemReadBinary(VOID *ip, VOID *addr) {
  putc('R', trace);
  fwrite(&addr, sizeof(VOID *), 1, trace);
}

// Print a memory write record
VOID RecordMemWriteBinary(VOID *ip, VOID *addr) {
  putc('W', trace);
  fwrite(&addr, sizeof(VOID *), 1, trace);
}

// Is called for every instruction and instruments reads and writes
VOID InstructionBinary(INS ins, VOID *v) {
  // Insert a call to printip before every instruction, and pass it the IP
  INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)RecordInstBinary, IARG_INST_PTR, IARG_END);

  // Instruments memory accesses using a predicated call, i.e.
  // the instrumentation is called iff the instruction will actually be executed.
  //
  // On the IA-32 and Intel(R) 64 architectures conditional moves and REP
  // prefixed instructions appear as predicated instructions in Pin.
  UINT32 memOperands = INS_MemoryOperandCount(ins);

  // Iterate over each memory operand of the instruction.
  for (UINT32 memOp = 0; memOp < memOperands; memOp++) {
    if (INS_MemoryOperandIsRead(ins, memOp)) {
      INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)RecordMemReadBinary, IARG_INST_PTR,
                               IARG_MEMORYOP_EA, memOp, IARG_END);
    }
    // Note that in some architectures a single memory operand can be
    // both read and written (for instance incl (%eax) on IA-32)
    // In that case we instrument it once for read and once for write.
    if (INS_MemoryOperandIsWritten(ins, memOp)) {
      INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)RecordMemWriteBinary, IARG_INST_PTR,
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
  fclose(trace);
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

  auto binary_mode = KnobBinary.Value();

  string file_name = KnobOutputFile.Value();
  if (!file_name.empty()) {
    trace = fopen(file_name.c_str(), binary_mode ? "wb" : "w");
  }
  else {
    // output to fd 3 by default.
    // output to stdout and stderr all might cause data loss (at least on PACE)
    trace = fdopen(3, binary_mode ? "wb" : "w");
  }

  INS_AddInstrumentFunction(binary_mode ? InstructionBinary : Instruction, 0);
  PIN_AddFiniFunction(Fini, 0);

  // Start the program, never returns
  PIN_StartProgram();

  return 0;
}
