#!/usr/bin/env python3

import os
import subprocess
from concurrent.futures import ThreadPoolExecutor

max_workers = 16
so_path =  '../src/intel_pin_tool/obj-intel64/pin_driver.so'
dir_workload = './apps/'
dir_out = './outputs/'
if not os.path.exists(dir_out): os.makedirs(dir_out)

workloads = ['seq-list', 'XSBench', 'BTree']
# sim_types = ['ice', 'con', 'uni-static', 'uni-dyn', 'uni-dyn-ind']
sim_types = ['con', 'uni-dyn']

def run_sim(sim_type, workload, path_out, **kwargs):
    run_args = ['pin', '-t', so_path, '-o', path_out, '-s', sim_type]
    run_args.extend((f'-{k} {v}' for k, v in kwargs))
    run_args.extend(['--', workload])
    subprocess.run(run_args)


with ThreadPoolExecutor(max_workers=max_workers) as executor:
    for sim_type in sim_types:
        for workload in workloads:
            path_out = f'{dir_out}exp_{workload}_{sim_type}.out'
            executor.submit(run_sim, sim_type, dir_workload + workload, path_out)
