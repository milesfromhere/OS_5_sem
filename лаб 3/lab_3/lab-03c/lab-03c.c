#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>

int main() {
    // Создание снимка процессов
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        printf("Error: Failed to create process snapshot. Error: %lu\n", GetLastError());
        return 1;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    printf("%-30s %-10s %-10s\n", "Process Name", "PID", "Parent PID");
    printf("--------------------------------------------------\n");

    // Получение первого процесса
    if (Process32First(hSnapshot, &pe32)) {
        do {
            printf("%-30s %-10lu %-10lu\n",
                pe32.szExeFile,
                pe32.th32ProcessID,
                pe32.th32ParentProcessID);
        } while (Process32Next(hSnapshot, &pe32));
    }
    else {
        printf("Error: Failed to get first process. Error: %lu\n", GetLastError());
    }

    CloseHandle(hSnapshot);
    return 0;
}