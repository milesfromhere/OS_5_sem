#include <windows.h>
#include <stdio.h>
#include <locale.h>
// -------- Global variables -------
// Global initialized variable (.data)
int g_init = 42;

// Global uninitialized variable (.bss)
int g_uninit;

// Global static initialized variable (.data)
static int g_static_init = 7;
static int g_static_uninit;

// Global constant string (.rdata)
const char* g_const_str = "Hello, sections!";
int add(int a, int b) {
    return a + b;
}

// Print label and address
void print_addr(const char* label, void* ptr) {
    printf("%-45s | address: %p\n", label, ptr);
}

int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "C");

    // Base address of the program
    HMODULE hModule = GetModuleHandleA(NULL);
    printf("Program base address (Lab-08a.exe): %p\n\n", hModule);

    // Local variables (stack)
    int local_init = 123;
    int local_uninit;
    local_uninit = 0;

    // Local static variables
    static int local_static_init = 555;
    static int local_static_uninit;

    // Function address (.text)
    void* fn_add_addr = (void*)&add;

    printf("Addresses of variables and functions:\n");

    print_addr("Global initialized variable (g_init)", &g_init);
    print_addr("Global uninitialized variable (g_uninit)", &g_uninit);
    print_addr("Global static initialized variable", &g_static_init);
    print_addr("Global static uninitialized variable", &g_static_uninit);
    print_addr("Global constant pointer (g_const_str)", (void*)&g_const_str);

    print_addr("Local initialized variable", &local_init);
    print_addr("Local uninitialized variable", &local_uninit);
    print_addr("Local static initialized variable", &local_static_init);
    print_addr("Local static uninitialized variable", &local_static_uninit);

    print_addr("Function argument argc (stack)", &argc);
    print_addr("Function argument argv (pointer)", &argv);

    if (argc > 0 && argv) {
        print_addr("argv[0] (path to executable)", argv[0]);
    }

    printf("%-45s | address: %p\n", "Function add (.text section)", fn_add_addr);
    printf("%-45s | address: %p\n", "g_const_str data (first character)", (const void*)g_const_str);

    printf("\nPress Enter to exit...\n");
    getchar();

    return 0;
}
