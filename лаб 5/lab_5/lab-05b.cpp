#include <windows.h>
#include <iostream>
#include <string>

using namespace std;

void PrintPriorityName(DWORD cls)
{
    switch (cls)
    {
    case REALTIME_PRIORITY_CLASS:      cout << "REALTIME"; break;
    case HIGH_PRIORITY_CLASS:          cout << "HIGH"; break;
    case ABOVE_NORMAL_PRIORITY_CLASS:  cout << "ABOVE NORMAL"; break;
    case NORMAL_PRIORITY_CLASS:        cout << "NORMAL"; break;
    case BELOW_NORMAL_PRIORITY_CLASS:  cout << "BELOW NORMAL"; break;
    case IDLE_PRIORITY_CLASS:          cout << "IDLE"; break;
    default:                           cout << "UNKNOWN"; break;
    }
}

DWORD ToPriorityClass(int p)
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

int main(int argc, char* argv[])
{
    setlocale(LC_ALL, "Rus");
    if (argc < 4)
    {
        cout << "Lab-05b: <P1_mask> <P2_priority> <P3_priority>\n";
        return 0;
    }

    DWORD mask = stoi(argv[1]);
    int p2 = stoi(argv[2]);
    int p3 = stoi(argv[3]);

    DWORD prio2 = ToPriorityClass(p2);
    DWORD prio3 = ToPriorityClass(p3);

    cout << "=== Параметры ===\n";
    cout << "Mask (P1): " << mask << "\n";
    cout << "P2 = "; PrintPriorityName(prio2); cout << "\n";
    cout << "P3 = "; PrintPriorityName(prio3); cout << "\n\n";

    STARTUPINFO si1 = { sizeof(STARTUPINFO) };
    PROCESS_INFORMATION pi1{};

    STARTUPINFO si2 = { sizeof(STARTUPINFO) };
    PROCESS_INFORMATION pi2{};

    // Используем wstring для Unicode
    wstring childCmd = L"C:\\Users\\nikit\\OneDrive\\Рабочий стол\\ОС\\лаб 5\\lab_5\\x64\\Debug\\lab_05x.exe";

    if (!CreateProcessW(
        NULL,
        &childCmd[0],
        NULL, NULL, FALSE,
        CREATE_NEW_CONSOLE | CREATE_SUSPENDED,
        NULL, NULL,
        &si1, &pi1
    ))
    {
        cerr << "Невозможно запустить 1-ый поток. Код = "
            << GetLastError() << endl;
        return 1;
    }

    wstring childCmd2 = L"C:\\Users\\nikit\\OneDrive\\Рабочий стол\\ОС\\лаб 5\\lab_5\\x64\\Debug\\lab_05x.exe";

    if (!CreateProcessW(
        NULL,
        &childCmd2[0],
        NULL, NULL, FALSE,
        CREATE_NEW_CONSOLE | CREATE_SUSPENDED,
        NULL, NULL,
        &si2, &pi2
    ))
    {
        cerr << "Невозможно запустить 2-ой поток. Код = "
            << GetLastError() << endl;
        return 1;
    }

    SetProcessAffinityMask(pi1.hProcess, mask);
    SetProcessAffinityMask(pi2.hProcess, mask);
    SetPriorityClass(pi1.hProcess, prio2);
    SetPriorityClass(pi2.hProcess, prio3);

    ResumeThread(pi1.hThread);
    ResumeThread(pi2.hThread);

    cout << "Процесс начался.\n";
    cout << "PID1 = " << pi1.dwProcessId << "\n";
    cout << "PID2 = " << pi2.dwProcessId << "\n\n";

    WaitForSingleObject(pi1.hProcess, INFINITE);
    WaitForSingleObject(pi2.hProcess, INFINITE);

    CloseHandle(pi1.hProcess);
    CloseHandle(pi1.hThread);
    CloseHandle(pi2.hProcess);
    CloseHandle(pi2.hThread);

    cout << endl << "Все потоки завершены." << endl;
    return 0;
}