#include <stdint.h>

bool is_prime(uint32_t n) {
    if (n <= 1) {
        return false;
    }
    if (n == 2) {
        return true;
    }
    if (n % 2 == 0) {
        return false;
    }

    for (uint32_t i = 3; i * i <= n; i += 2) {
        if (n % i == 0) {
            return false;
        }
    }
    return true;
}

uint32_t next_prime(uint32_t n)
{
    while (!is_prime(n)) n++;
    return n;
}