# Simulator Driver

### Setup

1. Download Intel Pin Tool.
2. Add pin's directory to PATH.
3. `cd` into this (`intel_pin_tool`) directory.

### Compile

(Replace `PIN_ROOT` with yours.)

```bash
mkdir obj-intel64
make PIN_ROOT=$HOME/src/pin/pin-3.28-98749-g6643ecee5-gcc-linux obj-intel64/pin_driver.so
```

### Usage

Invoke `pin_driver.so` as follows:

```bash
pin -t obj-intel64/pin_driver.so [simulator options] -- [benchmark program] [program args]
```
[simulator options] can be:

```
-s: simulator type, options are:
      ice: iceberg
      con: conventional
      uni-static: universal (static set-associative)
      uni-dyn-xor: universal (dynamic set-associative, xor-based hashing)
      uni-dyn-tbl: universal (dynamic set-associative, table-based hashing)
      uni-dyn-ind: universal (dynamic set-associative, independent hashing)
        
-m: memory size in MByte, can be a decimal
-w: for universal hashing: number of ways(banks)
-f: for iceberg hashing: frontyard size
-b: for iceberg hashing: backyard size

-o: output file name
-intvl: output satistics every n instructions; set to 0 to disable
-sep: output with thousands separators
```
