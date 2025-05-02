rm callgrind.*

make clean
make 
valgrind --tool=callgrind --toggle-collect=!rand ./main $1

kcachegrind $(find "." -type f -iname "callgrind.*")