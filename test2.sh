# set the partition where the job will run
#SBATCH --partition=halley

# The following line defines the name of the submitted job
#SBATCH --job-name=slurm_test

# The default output file if we run the script with the command sbatch test.sh
#SBATCH --output=2_out.txt

# set the number of nodes and processes per node
# That is, we will run this many tasks simultaneously
#SBATCH --nodes=2

# mail alert at start, end and abortion of execution
# The user will be mailed when the job starts and stops or aborts
# --mail-type=<type> where <type> may be BEGIN, END, FAIL, REQUEUE or ALL (for any change of job state)
#SBATCH --mail-type=ALL

# send mail to this address
#SBATCH --mail-user=<user_name>@ceng.metu.edu.tr

# Launch the command/application
mpiexec -n 2 ./hw1.o input.txt 1000000