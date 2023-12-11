#!/bin/bash
#SBATCH -CS7260_Project                         # Job name
#SBATCH -n1                                     # the number of tasks
#SBATCH --cpus-per-task=8
#SBATCH --mem-per-cpu=2G
#SBATCH -t480                                   # Duration of the job (Ex: 15 mins)
#SBATCH -oReport-%j.out                         # Combined output and error messages file
#SBATCH --mail-type=BEGIN,END,FAIL              # Mail preferences
#SBATCH --mail-user=zhao48@gatech.edu           # E-mail address for notifications
#SBATCH -C intel                                # Pin tool needs a intel CPU
cd $SLURM_SUBMIT_DIR                            # Change to working directory

module load cmake/3.23.1-327dbl gcc/12.1

export PATH=$HOME/src/pin/pin-3.28-98749-g6643ecee5-gcc-linux:$PATH

srun ./tool_expt_exec.py 8 graph500_512MB

