#!/bin/bash
#SBATCH -J hpc4nlp
#SBATCH --mail-user=glocalsaint@gmail.com
#SBATCH --mail-type=ALL
#SBATCH -e /home/vv52zasu/mpi/src/error32_all_fl.err
#SBATCH -o /home/vv52zasu/mpi/src/output32_all_fl.out
#SBATCH -n 32     # Number of tasks
#SBATCH -c 4 #Number of cores per process(task)
#SBATCH --mem-per-cpu=4000  # Main memory in MByte per MPI task
#SBATCH -t 600     # Hours, minutes and seconds, or '#SBATCH -t 10' - only minutes
cd /home/vv52zasu/mpi/src/
module load gcc openmpi/gcc boost
mpirun -np 32 hash
