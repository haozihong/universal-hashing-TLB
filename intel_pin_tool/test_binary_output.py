#!/usr/bin/env python3

import sys
from itertools import count

# f = open('ls.trace', 'rb')
f = sys.stdin.buffer

count_down = -1

for i in count(1):
    inst_b, addr_b = f.read(1), f.read(8)
    if inst_b == b'': break
    if inst_b not in {b'I', b'W', b'R'}:
        print('Error on line', i)
        if count_down < 0: count_down = 5
    if count_down >= 0:
        addr = int.from_bytes(addr_b, 'little')
        print(i, inst_b, hex(addr))
        count_down -= 1
        if count_down <= 0: break
