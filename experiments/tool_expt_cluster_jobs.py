#!/usr/bin/env python3

import os
import subprocess

so_path =  '../src/intel_pin_tool/obj-intel64/pin_driver.so'
dir_workload = './benchmarks/'
dir_out = './outputs/'
if not os.path.exists(dir_out): os.makedirs(dir_out)

with open('sb_bash_template.sbatch', 'r', encoding='utf-8') as f:
    sbatch_template = f.read()

sim_types = ["ice", "uni-static", "uni-dyn-ind", "uni-dyn-xor"]
sim_types = ["uni-dyn-xor"]

def run_sim(sim_type, workload, path_out, pin_args, workload_args):
    run_args = ['pin', '-t', so_path, '-o', path_out, '-s', sim_type]
    for k, v in pin_args.items():
        run_args.extend([f'-{k}', f'{v}'])
    run_args.extend(['--', workload])
    for k, v in workload_args.items():
        run_args.extend([f'-{k}', f'{v}'])
    # print('Executing: ', ' '.join(run_args))
    # subprocess.run(run_args)
    sbatch_script = sbatch_template + 'srun ' +  ' '.join(run_args)
    # print(sbatch_script)
    subprocess.run(['sbatch'], text=True, input=sbatch_script)

pin_args = {'intvl': 1_000_000_000}
workload = 'graph500/seq-csr'
workload_size = 18
for sim_type in sim_types:
    path_out = f'{dir_out}exp_{os.path.basename(workload)}_{workload_size}_{sim_type}.out'
    # workload_args = {'s': 22, 'e': 31 + 2 * workload_size}
    pin_args['m'] = 128
    workload_args = {'s': workload_size, 'e': workload_size}
    run_sim(sim_type, dir_workload + workload, path_out, pin_args, workload_args)

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
