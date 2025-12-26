#include <windows.h>
#include <stdio.h>

int main() {
    const char* programPath = "lab-03x.exe";

    PROCESS_INFORMATION infos[3];
    HANDLE handles[3];
    DWORD createdCount = 0;

    // Установить глобальную переменную ITER_NUM (для пользователя)
    // В коде это делается через системные настройки
    // Для демонстрации будем считать, что она установлена в значение 10

    // 1. Процесс: только имя файла (без аргументов)
    {
        STARTUPINFOA si;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        PROCESS_INFORMATION pi;
        ZeroMemory(&pi, sizeof(pi));

        printf("Creating process 1: application name only (should use global ITER_NUM=10)\n");

        if (CreateProcessA(
            programPath,
            NULL,
            NULL,
            NULL,
            FALSE,
            0,
            NULL,
            NULL,
            &si,
            &pi)) {
            printf("Process 1 created! PID=%lu\n", pi.dwProcessId);
            infos[createdCount] = pi;
            handles[createdCount] = pi.hProcess;
            ++createdCount;
        }
        else {
            printf("Process 1 failed! Error=%lu\n", GetLastError());
        }
    }

    // 2. Процесс: как в lab-03a процесс 2 (для сравнения)
    {
        STARTUPINFOA si;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        PROCESS_INFORMATION pi;
        ZeroMemory(&pi, sizeof(pi));

        char cmdLine[256];
        _snprintf_s(cmdLine, sizeof(cmdLine), _TRUNCATE, "\"%s\" %s", programPath, "15");

        printf("\nCreating process 2: with explicit argument 15\n");

        if (CreateProcessA(
            NULL,
            cmdLine,
            NULL,
            NULL,
            FALSE,
            0,
            NULL,
            NULL,
            &si,
            &pi)) {
            printf("Process 2 created! PID=%lu\n", pi.dwProcessId);
            infos[createdCount] = pi;
            handles[createdCount] = pi.hProcess;
            ++createdCount;
        }
        else {
            printf("Process 2 failed! Error=%lu\n", GetLastError());
        }
    }

    // 3. Процесс: локальная переменная окружения
    {
        STARTUPINFOA si;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        PROCESS_INFORMATION pi;
        ZeroMemory(&pi, sizeof(pi));

        // Локальная переменная окружения
        char envVars[] = "ITER_NUM=7\0";

        printf("\nCreating process 3: with local ITER_NUM=7\n");

        if (CreateProcessA(
            programPath,
            NULL,  // NULL вместо второго параметра
            NULL,
            NULL,
            FALSE,
            0,
            envVars, // Локальная переменная окружения
            NULL,
            &si,
            &pi)) {
            printf("Process 3 created! PID=%lu\n", pi.dwProcessId);
            infos[createdCount] = pi;
            handles[createdCount] = pi.hProcess;
            ++createdCount;
        }
        else {
            printf("Process 3 failed! Error=%lu\n", GetLastError());
        }
    }

    // Ожидание завершения
    if (createdCount > 0) {
        printf("\nWaiting for all child processes...\n");
        WaitForMultipleObjects(createdCount, handles, TRUE, INFINITE);
        printf("All child processes completed.\n");
    }

    // Освобождение ресурсов
    for (DWORD i = 0; i < createdCount; ++i) {
        CloseHandle(infos[i].hThread);
        CloseHandle(infos[i].hProcess);
    }

    return 0;
}