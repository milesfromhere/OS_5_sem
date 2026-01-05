#include <iostream>
#include <iomanip>
#include <dirent.h>
#include <sys/types.h>
#include <pwd.h>
#include <fstream>
#include <string>
#include <vector>

struct ProcessInfo {
    int pid;
    int ppid;
    std::string name;
};

std::vector<ProcessInfo> getProcessList() {
    std::vector<ProcessInfo> processes;
    DIR* procDir = opendir("/proc");
    
    if (!procDir) {
        perror("opendir /proc failed");
        return processes;
    }
    
    struct dirent* entry;
    while ((entry = readdir(procDir)) != nullptr) {
        // Проверяем, является ли entry числом (PID)
        if (entry->d_type == DT_DIR) {
            char* endptr;
            long pid = strtol(entry->d_name, &endptr, 10);
            if (*endptr == '\0') {
                // Чтение статуса процесса
                std::string statPath = std::string("/proc/") + entry->d_name + "/stat";
                std::ifstream statFile(statPath);
                if (statFile) {
                    std::string line;
                    std::getline(statFile, line);
                    
                    // Парсим строку статуса
                    size_t firstParen = line.find('(');
                    size_t lastParen = line.rfind(')');
                    if (firstParen != std::string::npos && lastParen != std::string::npos) {
                        ProcessInfo info;
                        info.pid = pid;
                        info.name = line.substr(firstParen + 1, lastParen - firstParen - 1);
                        
                        // Извлекаем PPID (4-е поле после имени)
                        std::string afterName = line.substr(lastParen + 2);
                        size_t pos = 0;
                        for (int i = 0; i < 2; i++) { // Пропускаем 2 поля
                            pos = afterName.find(' ', pos + 1);
                        }
                        if (pos != std::string::npos) {
                            size_t ppidStart = pos + 1;
                            size_t ppidEnd = afterName.find(' ', ppidStart);
                            info.ppid = std::stoi(afterName.substr(ppidStart, ppidEnd - ppidStart));
                            processes.push_back(info);
                        }
                    }
                }
            }
        }
    }
    
    closedir(procDir);
    return processes;
}

int main() {
    std::vector<ProcessInfo> processes = getProcessList();
    
    std::cout << std::left << std::setw(30) << "Process Name" 
              << std::setw(10) << "PID" 
              << std::setw(10) << "Parent PID" << std::endl;
    std::cout << std::string(50, '-') << std::endl;
    
    for (const auto& proc : processes) {
        std::cout << std::left << std::setw(30) << proc.name
                  << std::setw(10) << proc.pid
                  << std::setw(10) << proc.ppid << std::endl;
    }
    
    return 0;
}