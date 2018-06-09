git pull
mpic++ main.cpp
echo $1 $2 $3
mpirun -n 4 a.out $1 $2 $3