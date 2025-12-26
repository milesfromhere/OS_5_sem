#include <windows.h>
#include <shellapi.h>

static void WriteText(const char* text) {
    DWORD written;
    WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), text, lstrlenA(text), &written, NULL);
}

static void WriteNumber(int value) {
    char buf[16];
    int pos = sizeof(buf) - 1;
    buf[pos] = '\0';
    int v = value;
    if (v == 0) {
        buf[--pos] = '0';
    } else {
        if (v < 0) {
            v = -v;
        }
        while (v && pos > 0) {
            buf[--pos] = static_cast<char>('0' + (v % 10));
            v /= 10;
        }
        if (value < 0 && pos > 0) {
            buf[--pos] = '-';
        }
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

static bool IsPrime(int n) {
    if (n <= 1) return false;
    if (n <= 3) return true;
    if ((n % 2) == 0 || (n % 3) == 0) return false;
    for (int i = 5; i * i <= n; i += 6) {
        if ((n % i) == 0 || (n % (i + 2)) == 0) return false;
    }
    return true;
}

int main() {
    int argc = 0;
    LPWSTR* argvW = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (!argvW || argc != 3) {
        WriteText("Usage: Lab-03d-client <lower> <upper>\r\n");
        return 1;
    }

    char arg1[32];
    char arg2[32];
    WideCharToMultiByte(CP_ACP, 0, argvW[1], -1, arg1, sizeof(arg1), NULL, NULL);
    WideCharToMultiByte(CP_ACP, 0, argvW[2], -1, arg2, sizeof(arg2), NULL, NULL);
    if (argvW) LocalFree(argvW);

    int lower = ParseInt(arg1);
    int upper = ParseInt(arg2);
    if (lower >= upper) {
        WriteText("Error: lower must be less than upper\r\n");
        return 1;
    }

    WriteText("RANGE:");
    WriteNumber(lower);
    WriteText("-");
    WriteNumber(upper);
    WriteText(":");

    bool first = true;
    for (int i = lower; i <= upper; ++i) {
        if (IsPrime(i)) {
            if (!first) {
                WriteText(",");
            }
            WriteNumber(i);
            first = false;
        }
    }
    WriteText("\r\n");
    return 0;
}