#include <iostream>
#include <array>


#include "include/xxhash64.h"

int main(int argc, char *argv[]) {

    uint64_t a = 1;
    uint32_t result2 = XXHash64::hash(&a, 8, 0);

    std::cout << result2;
}