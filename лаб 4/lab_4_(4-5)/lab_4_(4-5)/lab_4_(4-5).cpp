#include "windows.h"
#include <iostream>
#include <thread>
#include <vector>
#include <string>

using namespace std;
#define MAX_THREADS 2
DWORD WINAPI Handle(LPVOID lpParm) {
    int n = ((int*)lpParm)[0];

    const char array[12] = {
    'u', 's', 'e', 'r',
    '7', '4', '8', 'c',
    '2', 'f', '7', '5'
    };
    for (int i = 0; i < n; i++) {
        int index = i % 12;
        printf("PID: %lu  TID: %lu  process %d - %c\n",
            GetCurrentProcessId(),
            GetCurrentThreadId(),
            i,
            array[index]);
        Sleep(300);
    }
    return 0;
}



int main() {
    cout << "PID of main process: " << GetCurrentProcessId() << endl;


    HANDLE threads[MAX_THREADS];
    int parm1 = 50, parm2 = 125, parm3 = 100;

    if (!(threads[0] = CreateThread(NULL, 0, Handle, &parm1, 0, NULL))) {
        cout << "Thread 1 : FAILURE\n";
        ExitProcess(1);
    }
    if (!(threads[1] = CreateThread(NULL, 0, Handle, &parm2, 0, NULL))) {
        cout << "Thread 2 : FAILURE\n";
        ExitProcess(1);
    }

    cout << "==== Main thread work =====\n";



    Handle(&parm3);

    cout << "\nWaiting for threads....\n";
    WaitForMultipleObjects(MAX_THREADS, threads, TRUE, INFINITE);

    cout << "\nWaiting for threads...\n";
    for (int i = 0; i < MAX_THREADS; i++) {
        CloseHandle(threads[i]);
    }

    cout << "==== All threads finished =====\n";
    return 0;
}
