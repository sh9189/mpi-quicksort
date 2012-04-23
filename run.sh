make; make clean all
./generate 8
mpirun -n 2 -machinefile mfile run
./validate
