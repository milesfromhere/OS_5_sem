#include <iostream>
#include <string>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>

int main(int argc, char* argv[]) {
    int iterations = 0;
    
    if (argc > 1) {
        iterations = std::atoi(argv[1]);
    }
    else {
        char* env_iter = std::getenv("ITER_NUM");
        if (env_iter != nullptr) {
            iterations = std::atoi(env_iter);
        }
        else {
            std::cerr << "Error: No iterations specified in command line or ITER_NUM environment variable" << std::endl;
            exit(1);
        }
    }
    
    if (iterations <= 0) {
        std::cerr << "Error: Invalid number of iterations: " << iterations << std::endl;
        exit(1);
    }
    
    std::cout << "Number of iterations: " << iterations << std::endl;
    std::cout << "PID: " << getpid() << std::endl;
    
    for (int i = 0; i < iterations; i++) {
        std::cout << "Iteration " << (i + 1) << ", PID: " << getpid() << std::endl;
        usleep(500000); // Задержка 500 мс (в микросекундах)
    }
    
    return 0;
}