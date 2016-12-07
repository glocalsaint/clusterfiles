#!/bin/bash
#SBATCH -J hpc4nlp
#SBATCH --mail-user=glocalsaint@gmail.com
#SBATCH --mail-type=ALL
#SBATCH -e /home/vv52zasu/mpi/src/error64_all_fl.err
#SBATCH -o /home/vv52zasu/mpi/src/output64_all_fl.out
#SBATCH -n 64     # Number of tasks
#SBATCH -c 2 #Number of cores per process(task)
#SBATCH --mem-per-cpu=4000  # Main memory in MByte per MPI task
#SBATCH -t 360     # Hours, minutes and seconds, or '#SBATCH -t 10' - only minutes
cd /home/vv52zasu/mpi/src/
module load gcc openmpi/gcc boost
mpirun -np 64 hash
