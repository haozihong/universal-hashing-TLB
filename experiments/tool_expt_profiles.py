#!/usr/bin/env python3

import os
import subprocess

so_path =  '../src/intel_pin_tool/obj-intel64/pin_driver.so'
dir_workload = './benchmarks/'
dir_out = './outputs/'

sim_types = ["ice", "uni-static", "uni-dyn-ind", "uni-dyn-xor"]

def sim_args(sim_type, workload, path_out, pin_args, workload_args):
    run_args = ['pin', '-t', so_path, '-o', path_out, '-s', sim_type]
    for k, v in pin_args.items():
        run_args.extend([f'-{k}', f'{v}'])
    run_args.extend(['--', workload])
    run_args.extend(workload_args)
    return run_args

def run_sim(sim_type, workload, path_out, pin_args, workload_args):
    run_args = sim_args(sim_type, workload, path_out, pin_args, workload_args)
    print('Executing: ', ' '.join(run_args))
    subprocess.run(run_args)

def args_set_graph500_64MB():
    workload = 'graph500/seq-csr'
    pin_args = {'intvl': 100_000_000, 'sep': 1, 'm': 64}
    s = 17
    args_set = []
    for e in range(14, 23, 2):
        for sim_type in sim_types:
            path_out = f'{dir_out}exp_{os.path.basename(workload)}_s{s}_e{e}_{pin_args["m"]}_{sim_type}.out'
            workload_args = ['-s', f'{s}', '-e', f'{e}']
            args_set.append(sim_args(sim_type, dir_workload + workload, path_out, pin_args, workload_args))
    return args_set

def args_set_graph500_512MB():
    workload = 'graph500/seq-csr'
    pin_args = {'intvl': 100_000_000, 'sep': 1, 'm': 512}
    s = 20
    args_set = []
    for e in (12, 15, 18, 21):
        for sim_type in sim_types:
            path_out = f'{dir_out}exp_{os.path.basename(workload)}_s{s}_e{e}_{pin_args["m"]}_{sim_type}.out'
            workload_args = ['-s', f'{s}', '-e', f'{e}']
            args_set.append(sim_args(sim_type, dir_workload + workload, path_out, pin_args, workload_args))
    return args_set

def args_set_BTree_1GB():
    workload = 'BTree/BTree'
    pin_args = {'intvl': 100_000_000, 'sep': 1, 'm': 1024}
    args_set = []
    for workload_size in range(11, 16):
        for sim_type in sim_types:
            path_out = f'{dir_out}exp_{os.path.basename(workload)}_{workload_size}M_{pin_args["m"]}_{sim_type}.out'
            workload_args = [f'{workload_size * 1_000_000}']
            args_set.append(sim_args(sim_type, dir_workload + workload, path_out, pin_args, workload_args))
    return args_set

def args_set_BTree_4GB():
    workload = 'BTree/BTree'
    pin_args = {'intvl': 100_000_000, 'sep': 1, 'm': 4096}
    args_set = []
    for workload_size in (37, 45, 53, 61):
        for sim_type in sim_types:
            path_out = f'{dir_out}exp_{os.path.basename(workload)}_{workload_size}M_{pin_args["m"]}_{sim_type}.out'
            workload_args = [f'{workload_size * 1_000_000}']
            args_set.append(sim_args(sim_type, dir_workload + workload, path_out, pin_args, workload_args))
    return args_set

def args_set_XSBench_128MB():
    workload = 'XSBench/XSBench'
    pin_args = {'intvl': 100_000_000, 'sep': 1, 'm': 128}
    p = 200_000
    args_set = []
    for g in (280, 330, 380):
        for sim_type in sim_types:
            path_out = f'{dir_out}exp_{os.path.basename(workload)}_g{g}_p{p}_{pin_args["m"]}_{sim_type}.out'
            workload_args = ['-t', '1', '-g', f'{g}', '-p', f'{p}']
            args_set.append(sim_args(sim_type, dir_workload + workload, path_out, pin_args, workload_args))
    return args_set

def args_set_XSBench_512MB():
    workload = 'XSBench/XSBench'
    pin_args = {'intvl': 100_000_000, 'sep': 1, 'm': 512}
    p = 500_000
    args_set = []
    for g in (900, 1100, 1300, 1500):
        for sim_type in sim_types:
            path_out = f'{dir_out}exp_{os.path.basename(workload)}_g{g}_p{p}_{pin_args["m"]}_{sim_type}.out'
            workload_args = ['-t', '1', '-g', f'{g}', '-p', f'{p}']
            args_set.append(sim_args(sim_type, dir_workload + workload, path_out, pin_args, workload_args))
    return args_set

def args_set_XSBench_1GB():
    workload = 'XSBench/XSBench'
    pin_args = {'intvl': 100_000_000, 'sep': 1, 'm': 1024}
    p = 500_000
    args_set = []
    for g in (1840, 2250, 2660, 3070):
        for sim_type in sim_types:
            path_out = f'{dir_out}exp_{os.path.basename(workload)}_g{g}_p{p}_{pin_args["m"]}_{sim_type}.out'
            workload_args = ['-t', '1', '-g', f'{g}', '-p', f'{p}']
            args_set.append(sim_args(sim_type, dir_workload + workload, path_out, pin_args, workload_args))
    return args_set

def args_set_XSBench_4GB():
    workload = 'XSBench/XSBench'
    pin_args = {'intvl': 100_000_000, 'sep': 1, 'm': 4096}
    p = 500_000
    args_set = []
    for g in (7370, 9010, 10650, 12290):
        for sim_type in sim_types:
            path_out = f'{dir_out}exp_{os.path.basename(workload)}_g{g}_p{p}_{pin_args["m"]}_{sim_type}.out'
            workload_args = ['-t', '1', '-g', f'{g}', '-p', f'{p}']
            args_set.append(sim_args(sim_type, dir_workload + workload, path_out, pin_args, workload_args))
    return args_set

def args_set_gups_2GB():
    workload = 'gups'
    workload_size = 2
    pin_args = {'intvl': 100_000_000, 'sep': 1, 'm': 1536}
    args_set = []
    for sim_type in sim_types:
        path_out = f'{dir_out}exp_{os.path.basename(workload)}_{workload_size}_{pin_args["m"]}_{sim_type}.out'
        workload_args = [f'{workload_size}']
        args_set.append(sim_args(sim_type, dir_workload + workload, path_out, pin_args, workload_args))

    pin_args = {'intvl': 100_000_000, 'sep': 1, 'm': 1792}
    for sim_type in sim_types:
        path_out = f'{dir_out}exp_{os.path.basename(workload)}_{workload_size}_{pin_args["m"]}_{sim_type}.out'
        workload_args = [f'{workload_size}']
        args_set.append(sim_args(sim_type, dir_workload + workload, path_out, pin_args, workload_args))
    return args_set

def args_set_gups_4GB():
    workload = 'gups'
    workload_size = 4
    args_set = []
    pin_args = {'intvl': 100_000_000, 'sep': 1, 'm': 4551}
    for sim_type in sim_types:
        path_out = f'{dir_out}exp_{os.path.basename(workload)}_{workload_size}_{pin_args["m"]}_{sim_type}.out'
        workload_args = [f'{workload_size}']
        args_set.append(sim_args(sim_type, dir_workload + workload, path_out, pin_args, workload_args))
    pin_args = {'intvl': 100_000_000, 'sep': 1, 'm': 3723.6}
    for sim_type in sim_types:
        path_out = f'{dir_out}exp_{os.path.basename(workload)}_{workload_size}_{pin_args["m"]}_{sim_type}.out'
        workload_args = [f'{workload_size}']
        args_set.append(sim_args(sim_type, dir_workload + workload, path_out, pin_args, workload_args))
    pin_args = {'intvl': 100_000_000, 'sep': 1, 'm': 3150.8}
    for sim_type in sim_types:
        path_out = f'{dir_out}exp_{os.path.basename(workload)}_{workload_size}_{pin_args["m"]}_{sim_type}.out'
        workload_args = [f'{workload_size}']
        args_set.append(sim_args(sim_type, dir_workload + workload, path_out, pin_args, workload_args))
    pin_args = {'intvl': 100_000_000, 'sep': 1, 'm': 2730.7}
    for sim_type in sim_types:
        path_out = f'{dir_out}exp_{os.path.basename(workload)}_{workload_size}_{pin_args["m"]}_{sim_type}.out'
        workload_args = [f'{workload_size}']
        args_set.append(sim_args(sim_type, dir_workload + workload, path_out, pin_args, workload_args))
    return args_set

expt_profiles = {
    'graph500_64MB': args_set_graph500_64MB,
    'graph500_512MB': args_set_graph500_512MB,
    'BTree_1GB': args_set_BTree_1GB,
    'BTree_4GB': args_set_BTree_4GB,
    'XSBench_128MB': args_set_XSBench_128MB,
    'XSBench_512MB': args_set_XSBench_512MB,
    'XSBench_1GB': args_set_XSBench_1GB,
    'XSBench_4GB': args_set_XSBench_4GB,
    'gups_2GB': args_set_gups_2GB,
    'gups_4GB': args_set_gups_4GB
}

