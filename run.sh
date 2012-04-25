make; make clean all
for((i=0;i<300;i++))
do
./generate 4194304 
mpirun -n 8 -machinefile mfile run >> result.txt
./validate
done
./results.py
