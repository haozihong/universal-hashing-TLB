#!/usr/bin/env python3

import os, sys
import subprocess
from concurrent.futures import ThreadPoolExecutor

max_workers = 16
try:
    max_workers = int(sys.argv[1])
except: pass

so_path =  '../src/intel_pin_tool/obj-intel64/pin_driver.so'
dir_workload = './benchmarks/'
dir_out = './outputs/'
if not os.path.exists(dir_out): os.makedirs(dir_out)

sim_types = ["ice", "uni-static", "uni-dyn-ind", "uni-dyn-xor"]

def run_sim(sim_type, workload, path_out, pin_args, workload_args):
    run_args = ['pin', '-t', so_path, '-o', path_out, '-s', sim_type]
    for k, v in pin_args.items():
        run_args.extend([f'-{k}', f'{v}'])
    run_args.extend(['--', workload])
    run_args.extend(workload_args)
    print('Executing: ', ' '.join(run_args))
    subprocess.run(run_args)

with ThreadPoolExecutor(max_workers=max_workers) as executor:
    workload = 'graph500/seq-csr'
    pin_args = {'intvl': 100_000_000, 'sep': 1, 'm': 64}
    s = 17
    for e in range(14, 23, 2):
        for sim_type in sim_types:
            path_out = f'{dir_out}exp_{os.path.basename(workload)}_s{s}_e{e}_{pin_args["m"]}_{sim_type}.out'
            workload_args = ['-s', f'{s}', '-e', f'{e}']
            executor.submit(run_sim, sim_type, dir_workload + workload, path_out, pin_args, workload_args)

    # workload = 'BTree/BTree'
    # workload_size = 6
    # pin_args = {'intvl': 100_000_000, 'sep': 1, 'm': 512}
    # for sim_type in sim_types:
    #     path_out = f'{dir_out}exp_{os.path.basename(workload)}_{workload_size}M_{pin_args["m"]}_{sim_type}.out'
    #     workload_args = [f'{workload_size * 1_000_000}']
    #     executor.submit(run_sim, sim_type, dir_workload + workload, path_out, pin_args, workload_args)

    # workload = 'BTree/BTree'
    # workload_size = 6
    # pin_args = {'intvl': 100_000_000, 'sep': 1, 'm': 256}
    # for sim_type in sim_types:
    #     path_out = f'{dir_out}exp_{os.path.basename(workload)}_{workload_size}M_{pin_args["m"]}_{sim_type}.out'
    #     workload_args = [f'{workload_size * 1_000_000}']
    #     executor.submit(run_sim, sim_type, dir_workload + workload, path_out, pin_args, workload_args)

    # workload = 'BTree/BTree'
    # workload_size = 12
    # pin_args = {'intvl': 100_000_000, 'sep': 1, 'm': 1024}
    # for sim_type in sim_types:
    #     path_out = f'{dir_out}exp_{os.path.basename(workload)}_{workload_size}M_{pin_args["m"]}_{sim_type}.out'
    #     workload_args = [f'{workload_size * 1_000_000}']
    #     executor.submit(run_sim, sim_type, dir_workload + workload, path_out, pin_args, workload_args)

    # workload = 'gups'
    # workload_size = 2
    # pin_args = {'intvl': 100_000_000, 'sep': 1, 'm': 1536}
    # for sim_type in sim_types:
    #     path_out = f'{dir_out}exp_{os.path.basename(workload)}_{workload_size}_{pin_args["m"]}_{sim_type}.out'
    #     workload_args = [f'{workload_size}']
    #     executor.submit(run_sim, sim_type, dir_workload + workload, path_out, pin_args, workload_args)

    # workload = 'gups'
    # workload_size = 2
    # pin_args = {'intvl': 100_000_000, 'sep': 1, 'm': 1792}
    # for sim_type in sim_types:
    #     path_out = f'{dir_out}exp_{os.path.basename(workload)}_{workload_size}_{pin_args["m"]}_{sim_type}.out'
    #     workload_args = [f'{workload_size}']
    #     executor.submit(run_sim, sim_type, dir_workload + workload, path_out, pin_args, workload_args)

