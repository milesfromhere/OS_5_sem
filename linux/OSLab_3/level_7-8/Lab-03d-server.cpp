#include <iostream>
#include <vector>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sstream>

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <process_count> <lower_bound> <upper_bound>" << std::endl;
        return 1;
    }
    
    int processCount = std::atoi(argv[1]);
    int lowerBound = std::atoi(argv[2]);
    int upperBound = std::atoi(argv[3]);
    
    if (processCount <= 0 || lowerBound >= upperBound) {
        std::cerr << "Error: Invalid parameters" << std::endl;
        return 1;
    }
    
    std::vector<pid_t> childPids;
    int range = upperBound - lowerBound + 1;
    int segmentSize = range / processCount;
    
    std::cout << "Starting " << processCount << " processes for range " 
              << lowerBound << "-" << upperBound << std::endl;
    
    for (int i = 0; i < processCount; i++) {
        // Расчет диапазона для текущего процесса
        int start = lowerBound + i * segmentSize;
        int end = (i == processCount - 1) ? upperBound : start + segmentSize - 1;
        
        pid_t pid = fork();
        
        if (pid == 0) {
            // Дочерний процесс
            std::stringstream ss;
            ss << "./Lab-03d-client " << start << " " << end;
            
            // Запуск клиента
            execlp("./Lab-03d-client", "./Lab-03d-client", 
                   std::to_string(start).c_str(), 
                   std::to_string(end).c_str(), 
                   nullptr);
            
            perror("execlp failed");
            exit(1);
        }
        else if (pid > 0) {
            std::cout << "Started process " << pid << " for range " 
                      << start << "-" << end << std::endl;
            childPids.push_back(pid);
        }
        else {
            std::cerr << "Failed to create process " << i + 1 << std::endl;
        }
    }
    
    // Ожидание завершения всех дочерних процессов
    std::cout << "Waiting for all child processes to complete..." << std::endl;
    for (pid_t pid : childPids) {
        int status;
        waitpid(pid, &status, 0);
        std::cout << "Process " << pid << " completed" << std::endl;
    }
    
    std::cout << "All processes completed." << std::endl;
    return 0;
}