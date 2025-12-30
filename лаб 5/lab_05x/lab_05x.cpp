#include <windows.h>
#include <iostream>
#include <ctime>
#include <string>

using namespace std;

const char* proc_priority_class_to_str(DWORD cls)
{
    switch (cls)
    {
    case IDLE_PRIORITY_CLASS: return "IDLE";
    case BELOW_NORMAL_PRIORITY_CLASS: return "BELOW_NORMAL";
    case NORMAL_PRIORITY_CLASS: return "NORMAL";
    case ABOVE_NORMAL_PRIORITY_CLASS: return "ABOVE_NORMAL";
    case HIGH_PRIORITY_CLASS: return "HIGH";
    case REALTIME_PRIORITY_CLASS: return "REALTIME";
    default: return "UNKNOWN";
    }
}

int main()
{
    setlocale(LC_ALL, "Rus");
    DWORD pid = GetCurrentProcessId();
    HANDLE hProcess = GetCurrentProcess();
    HANDLE hThread = GetCurrentThread();
    clock_t start = clock();

    for (int i = 1; i <= 1000000; i++)
    {
        if (i % 1000 == 0)
        {
            Sleep(200);

            DWORD tid = GetCurrentThreadId();
            DWORD procClass = GetPriorityClass(hProcess);
            int threadPrio = GetThreadPriority(hThread);
            DWORD cpu = GetCurrentProcessorNumber();

            cout << "Итерация: " << i << endl;
            cout << "PID: " << pid << endl;
            cout << "TID: " << tid << endl;
            cout << "Класс приоритета процесса: " << proc_priority_class_to_str(procClass) << endl;
            cout << "Приоритет потока: " << threadPrio << endl;
            cout << "Назначенный процессор: " << cpu << endl;
            cout << "------------------------------" << endl;
        }
    }

    clock_t end = clock();
    double time_sec = (double)(end - start) / CLOCKS_PER_SEC;
    cout << endl << "Время выполнения: " << time_sec << " сек" << endl;

    // ДОБАВЛЕНО: Пауза перед закрытием
    cout << endl << "Нажмите Enter для завершения программы..." << endl;

    // Очистка буфера ввода
    cin.clear();
    cin.ignore(cin.rdbuf()->in_avail());

    // Ожидание нажатия Enter
    string temp;
    getline(cin, temp);

    return 0;
}