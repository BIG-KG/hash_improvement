rm callgrind.*

make clean
make 
valgrind --tool=callgrind ./main

kcachegrind $(find "." -type f -iname "callgrind.*")