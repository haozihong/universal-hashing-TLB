### The code in this directory is no longer maintained

We found it very time-consuming to get the trace file first and then do the simulation. See `src/intel_pin_tool`.

### Setup

Add pin's directory to PATH.

`cd` into `intel_pin_tool` directory.

### Compile

(Replace `PIN_ROOT` with yours.)

```bash
make PIN_ROOT=$HOME/src/pin/pin-3.28-98749-g6643ecee5-gcc-linux obj-intel64/inst_mem_trace.so
```

### Use

Available flags
- `-o [file name]` - Specify the output file. Omit to output to `stdout``.
- `-binary` - Output the trace in binary stream. Format: access type (I, R, or W) of 1 byte and an address of 8 bytes (without Endian conversion, i.e., the same with the host machine). 

The trace will be output to `fd 3`.

When using a pipe, we need to redirect `fd 3` to `stdout` and `stdout` to `/dev/null`.

```bash
pin -t obj-intel64/inst_mem_trace.so -binary -- /bin/ls 3>&1 >&- >/dev/null | ./test_binary_output.py
```

Use a intermediate compressed file

```bash
pin -t obj-intel64/inst_mem_trace.so -binary -- /bin/ls 3>&1 >&- >/dev/null | xz -T 0 > ls.trace.xz
xz -dc ls.trace.xz | ./test_binary_output.py
```

