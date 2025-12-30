#include <windows.h>
#include <iostream>
#include <string>
#include <ctime>

using namespace std;

DWORD ToProcessPriority(int p)
{
    switch (p)
    {
    case 0: return IDLE_PRIORITY_CLASS;
    case 1: return BELOW_NORMAL_PRIORITY_CLASS;
    case 2: return NORMAL_PRIORITY_CLASS;
    case 3: return ABOVE_NORMAL_PRIORITY_CLASS;
    case 4: return HIGH_PRIORITY_CLASS;
    case 5: return REALTIME_PRIORITY_CLASS;
    default: return NORMAL_PRIORITY_CLASS;
    }
}

int ToThreadPriority(int p)
{
    switch (p)
    {
    case 0: return THREAD_PRIORITY_IDLE;
    case 1: return THREAD_PRIORITY_LOWEST;
    case 2: return THREAD_PRIORITY_BELOW_NORMAL;
    case 3: return THREAD_PRIORITY_NORMAL;
    case 4: return THREAD_PRIORITY_ABOVE_NORMAL;
    case 5: return THREAD_PRIORITY_HIGHEST;
    case 6: return THREAD_PRIORITY_TIME_CRITICAL;
    default: return THREAD_PRIORITY_NORMAL;
    }
}

struct ThreadData
{
    int id;
};

DWORD WINAPI WorkerThread(LPVOID arg)
{
    ThreadData* data = (ThreadData*)arg;

    clock_t start = clock();

    const int TOTAL_ITERS = 1000000;

    for (int i = 1; i <= TOTAL_ITERS; i++)
    {
        if (i % 1000 == 0)
        {
            Sleep(200);

            cout << "[Поток " << data->id << "] Итерация = " << i << endl
                << "  PID:            " << GetCurrentProcessId() << endl
                << "  TID:            " << GetCurrentThreadId() << endl
                << "  Класс приоритета процесса: " << GetPriorityClass(GetCurrentProcess()) << endl
                << "  Приоритет потока:          " << GetThreadPriority(GetCurrentThread()) << endl
                << "  Номер процессора:          " << GetCurrentProcessorNumber() << endl
                << "--------------------------------------" << endl;
        }
    }

    double secs = double(clock() - start) / CLOCKS_PER_SEC;
    cout << "[Поток " << data->id << "] Завершён за " << secs << " сек" << endl;

    return 0;
}

int main(int argc, char* argv[])
{
    setlocale(LC_ALL, "Rus");

    if (argc < 5)
    {
        cout << "Lab-05c.exe <Маска_процессора> <Приоритет_процесса> <Приоритет_поток1> <Приоритет_поток2>" << endl;
        return 0;
    }

    DWORD mask = stoi(argv[1]);
    int p2 = stoi(argv[2]);
    int p3 = stoi(argv[3]);
    int p4 = stoi(argv[4]);

    DWORD procPrio = ToProcessPriority(p2);
    int thrPrio1 = ToThreadPriority(p3);
    int thrPrio2 = ToThreadPriority(p4);

    cout << "=== Параметры запуска ===" << endl;
    cout << "Маска процессора (P1): " << mask << endl;
    cout << "Класс приоритета процесса (P2): " << procPrio << endl;
    cout << "Приоритет потока 1 (P3): " << thrPrio1 << endl;
    cout << "Приоритет потока 2 (P4): " << thrPrio2 << endl << endl;

    SetProcessAffinityMask(GetCurrentProcess(), mask);
    SetPriorityClass(GetCurrentProcess(), procPrio);

    ThreadData d1{ 1 };
    ThreadData d2{ 2 };

    HANDLE hThread1 = CreateThread(NULL, 0, WorkerThread, &d1, CREATE_SUSPENDED, NULL);
    HANDLE hThread2 = CreateThread(NULL, 0, WorkerThread, &d2, CREATE_SUSPENDED, NULL);

    SetThreadPriority(hThread1, thrPrio1);
    SetThreadPriority(hThread2, thrPrio2);

    ResumeThread(hThread1);
    ResumeThread(hThread2);

    WaitForSingleObject(hThread1, INFINITE);
    WaitForSingleObject(hThread2, INFINITE);

    CloseHandle(hThread1);
    CloseHandle(hThread2);

    cout << endl << "Все потоки завершены." << endl;

    return 0;
}
