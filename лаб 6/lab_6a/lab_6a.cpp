#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <windows.h>
#include <stdio.h>

CRITICAL_SECTION criticalSection;

struct ThreadParameters
{
    int iterationCount;
    const wchar_t* threadIdentifier;
};

DWORD WINAPI ThreadWorker(LPVOID parameter)
{
    ThreadParameters* threadData = (ThreadParameters*)parameter;
    int iterationCount = threadData->iterationCount;
    const wchar_t* threadIdentifier = threadData->threadIdentifier;

    wchar_t userName[256];
    DWORD userNameLength = GetEnvironmentVariableW(L"USERNAME", userName, 256);
    if (userNameLength == 0 || userNameLength >= 256) wcscpy(userName, L"Unknown");
    int userNameSize = wcslen(userName);

    for (int iteration = 1; iteration <= iterationCount; iteration++)
    {
        if (iteration == 30)
            EnterCriticalSection(&criticalSection);

        wprintf(L"Поток=%ls  Итерация=%d  Символ=%lc\n",
            threadIdentifier,
            iteration,
            userName[(iteration - 1) % userNameSize]);

        if (iteration == 60)
            LeaveCriticalSection(&criticalSection);
        Sleep(100);
    }

    return 0;
}

int wmain()
{
    setlocale(LC_ALL, "Rus");

    InitializeCriticalSection(&criticalSection);

    HANDLE threadHandles[2];

    ThreadParameters threadParamA{ 90, L"A" };
    ThreadParameters threadParamB{ 90, L"B" };
    ThreadParameters threadParamMain{ 90, L"M" };

    threadHandles[0] = CreateThread(NULL, 0, ThreadWorker, &threadParamA, 0, NULL);
    threadHandles[1] = CreateThread(NULL, 0, ThreadWorker, &threadParamB, 0, NULL);

    if (!threadHandles[0] || !threadHandles[1])
    {
        wprintf(L"Ошибка создания потоков!\n");
        return 1;
    }

    wprintf(L"Созданы потоки A и B.\n\n");

    ThreadWorker(&threadParamMain);

    WaitForMultipleObjects(2, threadHandles, TRUE, INFINITE);

    CloseHandle(threadHandles[0]);
    CloseHandle(threadHandles[1]);

    DeleteCriticalSection(&criticalSection);

    wprintf(L"\nВсе потоки завершены.\n");

    return 0;
}
