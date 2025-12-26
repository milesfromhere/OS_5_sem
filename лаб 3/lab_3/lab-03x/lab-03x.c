#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

static int ParseInt(const char* s) {
    int v = 0;
    if (!s) return 0;
    while (*s && *s == ' ') ++s;
    while (*s >= '0' && *s <= '9') {
        v = v * 10 + (*s - '0');
        ++s;
    }
    return v;
}

int main(int argc, char* argv[]) {
    int iterations = 0;
    char* env_iter = NULL;
    PROCESS_INFORMATION* processes = NULL;
    HANDLE* handles = NULL;
    int num_processes = 0;
    char command_line[256];

    // Получение количества итераций из аргументов командной строки
    if (argc > 1) {
        iterations = ParseInt(argv[1]);
    }
    else {
        // Если аргументов нет, пытаемся получить из переменной окружения
        env_iter = getenv("ITER_NUM");
        if (env_iter != NULL) {
            iterations = ParseInt(env_iter);
        }
        else {
            printf("Error: No iterations specified in command line or ITER_NUM environment variable\n");
            ExitProcess(1);
        }
    }

    if (iterations <= 0) {
        printf("Error: Invalid number of iterations: %d\n", iterations);
        ExitProcess(1);
    }

    printf("%d iterations at all\n", iterations);

    // Создаем процессы
    processes = (PROCESS_INFORMATION*)malloc(iterations * sizeof(PROCESS_INFORMATION));
    handles = (HANDLE*)malloc(iterations * sizeof(HANDLE));

    if (!processes || !handles) {
        printf("Error: Memory allocation failed\n");
        ExitProcess(1);
    }

    // Получаем путь к текущему исполняемому файлу
    char exe_path[MAX_PATH];
    GetModuleFileName(NULL, exe_path, MAX_PATH);

    for (int i = 0; i < iterations; i++) {
        STARTUPINFO si;
        PROCESS_INFORMATION pi;

        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));

        // Формируем командную строку: программа запускает себя с аргументом 1
        snprintf(command_line, sizeof(command_line), "\"%s\" 1", exe_path);

        // Создаем процесс
        if (!CreateProcess(
            NULL,           // Имя исполняемого модуля (используется command_line)
            command_line,   // Командная строка
            NULL,           // Атрибуты защиты процесса
            NULL,           // Атрибуты защиты потока
            FALSE,          // Наследование дескрипторов
            0,              // Флаги создания
            NULL,           // Окружение
            NULL,           // Текущий каталог
            &si,            // STARTUPINFO
            &pi             // PROCESS_INFORMATION
        )) {
            printf("Error: Failed to create process %d (Error: %lu)\n", i, GetLastError());
            continue;
        }

        processes[num_processes] = pi;
        handles[num_processes] = pi.hProcess;
        num_processes++;

        // Закрываем ненужный дескриптор потока
        CloseHandle(pi.hThread);
    }

    // Ждем завершения всех созданных процессов
    WaitForMultipleObjects(num_processes, handles, TRUE, INFINITE);

    // Закрываем дескрипторы процессов
    for (int i = 0; i < num_processes; i++) {
        CloseHandle(handles[i]);
    }

    free(processes);
    free(handles);

    return 0;
}