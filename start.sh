rm callgrind.*

make clean
make 
valgrind --tool=callgrind ./main $1

kcachegrind $(find "." -type f -iname "callgrind.*")