#!/usr/bin/env python3

import os, sys
import subprocess
from concurrent.futures import ThreadPoolExecutor

from tool_expt_profiles import expt_profiles, dir_out

max_workers = 16
try:
    max_workers = int(sys.argv[1])
except: pass

if not os.path.exists(dir_out): os.makedirs(dir_out)

args_set = []
for arg in sys.argv[1:]:
    if arg in expt_profiles:
        args_set.extend(expt_profiles[arg]())

with ThreadPoolExecutor(max_workers=max_workers) as executor:
    for run_args in args_set:
        executor.submit(subprocess.run, run_args)

