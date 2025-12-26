#include <windows.h>
#include <stdio.h>

int main() {
    const char* programPath = "lab-03x.exe";
    const char* iterations = "5";

    PROCESS_INFORMATION infos[3];
    HANDLE handles[3];
    DWORD createdCount = 0;

    // 1. Процесс с аргументом в lpApplicationName - ДОЛЖЕН ЗАВАЛИТЬСЯ
    {
        STARTUPINFOA si;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        PROCESS_INFORMATION pi;
        ZeroMemory(&pi, sizeof(pi));

        // Неправильный способ: аргумент в lpApplicationName
        char wrongAppName[256];
        _snprintf_s(wrongAppName, sizeof(wrongAppName), _TRUNCATE, "%s %s", programPath, iterations);

        printf("Creating process 1: lpApplicationName with argument (SHOULD FAIL)\n");

        if (CreateProcessA(
            wrongAppName,    // НЕПРАВИЛЬНО: аргумент в имени приложения
            NULL,
            NULL,
            NULL,
            FALSE,
            0,
            NULL,
            NULL,
            &si,
            &pi)) {
            printf("ERROR: Process 1 was created but shouldn't be! PID=%lu\n", pi.dwProcessId);
            infos[createdCount] = pi;
            handles[createdCount] = pi.hProcess;
            ++createdCount;
        }
        else {
            DWORD err = GetLastError();
            printf("Process 1 failed as expected. Error=%lu\n", err);
        }
    }

    // 2. Процесс только с lpCommandLine - ДОЛЖЕН РАБОТАТЬ
    {
        STARTUPINFOA si;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        PROCESS_INFORMATION pi;
        ZeroMemory(&pi, sizeof(pi));

        char cmdLine[256];
        _snprintf_s(cmdLine, sizeof(cmdLine), _TRUNCATE, "\"%s\" %s", programPath, iterations);

        printf("\nCreating process 2: only lpCommandLine (SHOULD WORK)\n");

        if (CreateProcessA(
            NULL,
            cmdLine,        // Полная командная строка
            NULL,
            NULL,
            FALSE,
            0,
            NULL,
            NULL,
            &si,
            &pi)) {
            printf("Process 2 created successfully! PID=%lu\n", pi.dwProcessId);
            infos[createdCount] = pi;
            handles[createdCount] = pi.hProcess;
            ++createdCount;
        }
        else {
            printf("ERROR: Process 2 failed! Error=%lu\n", GetLastError());
        }
    }

    // 3. Процесс с обоими параметрами - ДОЛЖЕН РАБОТАТЬ
    {
        STARTUPINFOA si;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        PROCESS_INFORMATION pi;
        ZeroMemory(&pi, sizeof(pi));

        char cmdLineWithSpace[32];
        _snprintf_s(cmdLineWithSpace, sizeof(cmdLineWithSpace), _TRUNCATE, " %s", iterations);

        printf("\nCreating process 3: lpApplicationName + lpCommandLine with space (SHOULD WORK)\n");

        if (CreateProcessA(
            programPath,    // Только имя программы
            cmdLineWithSpace, // Аргументы с пробелом в начале
            NULL,
            NULL,
            FALSE,
            0,
            NULL,
            NULL,
            &si,
            &pi)) {
            printf("Process 3 created successfully! PID=%lu\n", pi.dwProcessId);
            infos[createdCount] = pi;
            handles[createdCount] = pi.hProcess;
            ++createdCount;
        }
        else {
            printf("ERROR: Process 3 failed! Error=%lu\n", GetLastError());
        }
    }

    // Ожидание завершения всех успешно созданных процессов
    if (createdCount > 0) {
        printf("\nWaiting for all child processes...\n");
        WaitForMultipleObjects(createdCount, handles, TRUE, INFINITE);
        printf("All child processes completed.\n");
    }
    else {
        printf("\nNo child processes were created.\n");
    }

    // Освобождение ресурсов
    for (DWORD i = 0; i < createdCount; ++i) {
        CloseHandle(infos[i].hThread);
        CloseHandle(infos[i].hProcess);
    }

    return 0;
}