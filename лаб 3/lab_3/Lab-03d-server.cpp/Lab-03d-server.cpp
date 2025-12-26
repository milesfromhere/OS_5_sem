#include <windows.h>
#include <shellapi.h>

static void WriteText(const char* text) {
    DWORD out;
    WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), text, lstrlenA(text), &out, NULL);
}

static void WriteNumber(int value) {
    char buf[16];
    int pos = sizeof(buf) - 1;
    buf[pos] = '\0';
    int v = value;
    if (v == 0) {
        buf[--pos] = '0';
    } else {
        if (v < 0) { v = -v; }
        while (v && pos > 0) {
            buf[--pos] = static_cast<char>('0' + (v % 10));
            v /= 10;
        }
        if (value < 0 && pos > 0) buf[--pos] = '-';
    }
    WriteText(&buf[pos]);
}

static int ParseInt(const char* s) {
    int sign = 1;
    int v = 0;
    while (*s == ' ') ++s;
    if (*s == '-') { sign = -1; ++s; }
    while (*s >= '0' && *s <= '9') {
        v = v * 10 + (*s - '0');
        ++s;
    }
    return v * sign;
}

int main() {
    int argc = 0;
    LPWSTR* argvW = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (!argvW || argc != 4) {
        WriteText("Usage: Lab-03d-server <proc_count> <low> <up>\r\n");
        return 1;
    }

    char arg1[32], arg2[32], arg3[32];
    WideCharToMultiByte(CP_ACP, 0, argvW[1], -1, arg1, sizeof(arg1), NULL, NULL);
    WideCharToMultiByte(CP_ACP, 0, argvW[2], -1, arg2, sizeof(arg2), NULL, NULL);
    WideCharToMultiByte(CP_ACP, 0, argvW[3], -1, arg3, sizeof(arg3), NULL, NULL);
    if (argvW) LocalFree(argvW);

    int procCount = ParseInt(arg1);
    int lower = ParseInt(arg2);
    int upper = ParseInt(arg3);
    if (procCount <= 0 || lower >= upper) {
        WriteText("Error: invalid parameters\r\n");
        return 1;
    }

    int range = upper - lower + 1;
    int segment = range / procCount;

    PROCESS_INFORMATION* infos = static_cast<PROCESS_INFORMATION*>(
        LocalAlloc(LPTR, sizeof(PROCESS_INFORMATION) * procCount));
    HANDLE* reads = static_cast<HANDLE*>(LocalAlloc(LPTR, sizeof(HANDLE) * procCount));
    HANDLE* procs = static_cast<HANDLE*>(LocalAlloc(LPTR, sizeof(HANDLE) * procCount));
    if (!infos || !reads || !procs) {
        WriteText("Error: allocation failed\r\n");
        return 1;
    }

    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;

    for (int i = 0; i < procCount; ++i) {
        int start = lower + i * segment;
        int end = (i == procCount - 1) ? upper : start + segment - 1;

        HANDLE hRead, hWrite;
        if (!CreatePipe(&hRead, &hWrite, &sa, 0)) {
            WriteText("CreatePipe failed\r\n");
            continue;
        }
        SetHandleInformation(hRead, HANDLE_FLAG_INHERIT, 0);

        char cmdLine[128];
        char startBuf[16];
        char endBuf[16];
        wsprintfA(startBuf, "%d", start);
        wsprintfA(endBuf, "%d", end);
        lstrcpyA(cmdLine, "Lab-03d-client.exe ");
        lstrcatA(cmdLine, startBuf);
        lstrcatA(cmdLine, " ");
        lstrcatA(cmdLine, endBuf);

        STARTUPINFOA si;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESTDHANDLES;
        si.hStdOutput = hWrite;
        si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
        si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);

        PROCESS_INFORMATION pi;
        ZeroMemory(&pi, sizeof(pi));

        WriteText("Starting: ");
        WriteText(cmdLine);
        WriteText("\r\n");

        if (CreateProcessA(
                NULL,
                cmdLine,
                NULL,
                NULL,
                TRUE,           // наследуем канал
                0,
                NULL,
                NULL,
                &si,
                &pi)) {
            CloseHandle(pi.hThread);
            CloseHandle(hWrite); // закрываем пишущую сторону у родителя
            reads[i] = hRead;
            procs[i] = pi.hProcess;
            infos[i] = pi;
        } else {
            WriteText("CreateProcess failed. Err=");
            WriteNumber(GetLastError());
            WriteText("\r\n");
            CloseHandle(hRead);
            CloseHandle(hWrite);
            procs[i] = NULL;
            reads[i] = NULL;
        }
    }

    char buffer[256];
    for (int i = 0; i < procCount; ++i) {
        if (reads[i]) {
            DWORD read = 0;
            while (ReadFile(reads[i], buffer, sizeof(buffer) - 1, &read, NULL) && read > 0) {
                buffer[read] = '\0';
                WriteText(buffer);
            }
            CloseHandle(reads[i]);
        }
    }

    for (int i = 0; i < procCount; ++i) {
        if (procs[i]) {
            WaitForSingleObject(procs[i], INFINITE);
            CloseHandle(procs[i]);
        }
    }

    LocalFree(infos);
    LocalFree(reads);
    LocalFree(procs);

    WriteText("All processes completed.\r\n");
    return 0;
}