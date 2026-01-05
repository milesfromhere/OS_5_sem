#include <iostream>
#include <string>
#include <vector>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cstring>

int main() {
    const std::string programPath = "./Lab-03x";
    const std::string iterations = "5";
    
    std::vector<pid_t> childPids;
    
    // Процесс 1: использование execvp с аргументами
    {
        pid_t pid = fork();
        if (pid == 0) {
            // Дочерний процесс
            std::cout << "Creating process 1 with arguments" << std::endl;
            char* args[] = {
                const_cast<char*>("./Lab-03x"),
                const_cast<char*>("5"),
                nullptr
            };
            execvp(args[0], args);
            perror("execvp failed");
            exit(1);
        }
        else if (pid > 0) {
            std::cout << "Process 1 created successfully. PID: " << pid << std::endl;
            childPids.push_back(pid);
        }
        else {
            std::cerr << "Failed to create process 1" << std::endl;
        }
    }
    
    // Процесс 2: использование execlp
    {
        pid_t pid = fork();
        if (pid == 0) {
            // Дочерний процесс
            std::cout << "Creating process 2 with execlp" << std::endl;
            execlp("./Lab-03x", "./Lab-03x", "5", nullptr);
            perror("execlp failed");
            exit(1);
        }
        else if (pid > 0) {
            std::cout << "Process 2 created successfully. PID: " << pid << std::endl;
            childPids.push_back(pid);
        }
        else {
            std::cerr << "Failed to create process 2" << std::endl;
        }
    }
    
    // Процесс 3: с переменной окружения
    {
        pid_t pid = fork();
        if (pid == 0) {
            // Дочерний процесс
            std::cout << "Creating process 3 with environment" << std::endl;
            setenv("ITER_NUM", "5", 1);
            execlp("./Lab-03x", "./Lab-03x", nullptr);
            perror("execlp failed");
            exit(1);
        }
        else if (pid > 0) {
            std::cout << "Process 3 created successfully. PID: " << pid << std::endl;
            childPids.push_back(pid);
        }
        else {
            std::cerr << "Failed to create process 3" << std::endl;
        }
    }
    
    // Ожидание завершения всех дочерних процессов
    std::cout << "Waiting for all child processes to complete..." << std::endl;
    for (pid_t pid : childPids) {
        int status;
        waitpid(pid, &status, 0);
        std::cout << "Process " << pid << " completed" << std::endl;
    }
    
    std::cout << "All child processes completed." << std::endl;
    return 0;
}