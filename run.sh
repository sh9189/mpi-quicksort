make; make clean all
./generate 1048576
mpirun -n 4 -machinefile mfile run
./validate
