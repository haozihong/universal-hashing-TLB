#!/usr/bin/env python3

import sys

# f = open('ls.trace', 'rb')
f = sys.stdin.buffer

for _ in range(4):
    inst_b, addr_b = f.read(1), f.read(8)
    print(inst_b.decode())
    addr = int.from_bytes(addr_b, 'little')
    print(hex(addr))
