#include <iostream>
#include <vector>
#include <cmath>
#include <unistd.h>

bool isPrime(int n) {
    if (n <= 1) return false;
    if (n <= 3) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;
    
    for (int i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0)
            return false;
    }
    return true;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <lower_bound> <upper_bound>" << std::endl;
        return 1;
    }
    
    int lower = std::atoi(argv[1]);
    int upper = std::atoi(argv[2]);
    
    if (lower >= upper) {
        std::cerr << "Error: Lower bound must be less than upper bound" << std::endl;
        return 1;
    }
    
    std::vector<int> primes;
    
    // Поиск простых чисел в заданном диапазоне
    for (int i = lower; i <= upper; i++) {
        if (isPrime(i)) {
            primes.push_back(i);
        }
    }
    
    // Вывод результатов в stdout (будет перенаправлен в канал)
    std::cout << "Range " << lower << "-" << upper << ": ";
    for (int prime : primes) {
        std::cout << prime << " ";
    }
    std::cout << std::endl;
    
    return 0;
}