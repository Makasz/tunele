echo $1 $2 $3
if [ -z "$1" ] || [ -z "$2" ] || [ -z "$3" ]
  then
    echo "Pass arguments in format: <max_trip_size> <size_of_subspace> <debug_mode 0/1>"
    exit 1
fi
git pull
mpic++ main.cpp
mpirun -n 20 --hostfile $4 a.out $1 $2 $3
