#include <iostream>
#include <string>
#include <vector>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {
    std::vector<pid_t> childPids;
    
    // Процесс 1: только глобальная переменная окружения
    {
        pid_t pid = fork();
        if (pid == 0) {
            std::cout << "Process 1: using global ITER_NUM" << std::endl;
            execlp("./Lab-03x", "./Lab-03x", nullptr);
            perror("execlp failed");
            exit(1);
        }
        else if (pid > 0) {
            std::cout << "Process 1 created. PID: " << pid << std::endl;
            childPids.push_back(pid);
        }
    }
    
    // Процесс 2: с аргументом командной строки
    {
        pid_t pid = fork();
        if (pid == 0) {
            std::cout << "Process 2: using command line argument" << std::endl;
            execlp("./Lab-03x", "./Lab-03x", "5", nullptr);
            perror("execlp failed");
            exit(1);
        }
        else if (pid > 0) {
            std::cout << "Process 2 created. PID: " << pid << std::endl;
            childPids.push_back(pid);
        }
    }
    
    // Процесс 3: с локальной переменной окружения
    {
        pid_t pid = fork();
        if (pid == 0) {
            std::cout << "Process 3: using local ITER_NUM=3" << std::endl;
            setenv("ITER_NUM", "3", 1);
            execlp("./Lab-03x", "./Lab-03x", nullptr);
            perror("execlp failed");
            exit(1);
        }
        else if (pid > 0) {
            std::cout << "Process 3 created. PID: " << pid << std::endl;
            childPids.push_back(pid);
        }
    }
    
    // Ожидание завершения
    for (pid_t pid : childPids) {
        int status;
        waitpid(pid, &status, 0);
    }
    
    std::cout << "All processes completed." << std::endl;
    return 0;
}