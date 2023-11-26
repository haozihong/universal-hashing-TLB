#!/usr/bin/env python3

import os
import subprocess
from concurrent.futures import ThreadPoolExecutor

max_workers = 1
so_path =  '../src/intel_pin_tool/obj-intel64/pin_driver.so'
dir_workload = './benchmarks/'
dir_out = './outputs/'
if not os.path.exists(dir_out): os.makedirs(dir_out)

# workloads = ['graph500/seq-csr']
sim_types = ["ice", "uni-static", "uni-dyn-ind", "uni-dyn-tbl", "uni-dyn-xor"]

def run_sim(sim_type, workload, path_out, pin_args, workload_args):
    run_args = ['pin', '-t', so_path, '-o', path_out, '-s', sim_type]
    for k, v in pin_args.items():
        run_args.extend([f'-{k}', f'{v}'])
    run_args.extend(['--', workload])
    for k, v in workload_args.items():
        run_args.extend([f'-{k}', f'{v}'])
    print('Executing: ', ' '.join(run_args))
    subprocess.run(run_args)

with ThreadPoolExecutor(max_workers=max_workers) as executor:
    pin_args = {'intvl': 1_000_000_000}
    workload = 'graph500/seq-csr'
    workload_size = 4
    for sim_type in sim_types:
        path_out = f'{dir_out}exp_{os.path.basename(workload)}_{workload_size}_{sim_type}.out'
        # workload_args = {'s': 22, 'e': 31 + 2 * workload_size}
        pin_args['m'] = 64
        workload_args = {'s': 17, 'e': 17}
        executor.submit(run_sim, sim_type, dir_workload + workload, path_out, pin_args, workload_args)

    # pin_args = {'intvl': 10_000_000_000}
    # workload = 'XSBench/XSBench'
    # workload_size = 4
    # for sim_type in sim_types:
    #     path_out = f'{dir_out}exp_{os.path.basename(workload)}_{workload_size}_{sim_type}.out'
    #     workload_args = {'t': 1, 'g': 8313 + 512 * workload_size, 'p': 500_000}
    #     executor.submit(run_sim, sim_type, dir_workload + workload, path_out, pin_args, workload_args)

    # pin_args = {'intvl': 10_000_000_000}
    # workload = 'XSBench/XSBench'
    # workload_size = 4
    # for sim_type in sim_types:
    #     path_out = f'{dir_out}exp_{os.path.basename(workload)}_{workload_size}_{sim_type}.out'
    #     workload_args = {'t': 1, 'g': 8313 + 512 * workload_size, 'p': 500_000}
    #     executor.submit(run_sim, sim_type, dir_workload + workload, path_out, pin_args, workload_args)
