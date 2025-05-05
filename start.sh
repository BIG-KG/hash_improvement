rm callgrind.*

make clean
make 
sudo cpupower frequency-set -g userspace -f 1.2GHz
valgrind --tool=callgrind --toggle-collect=!rand ./main $1 $2 $3
sudo cpupower frequency-set -g ondemand

kcachegrind $(find "." -type f -iname "callgrind.*")