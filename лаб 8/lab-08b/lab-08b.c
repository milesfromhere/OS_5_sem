#include <windows.h>
#include <windows.h>
#include <stdio.h>
#include <locale.h>

int main() {
    setlocale(LC_ALL, "C");

    // 1. Get system page size
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    DWORD pageSize = si.dwPageSize;
    printf("Page size: %lu bytes\n", (unsigned long)pageSize);

    // 2. Reserve 256 pages of virtual memory
    SIZE_T reserveSize = 256 * pageSize;
    LPVOID baseAddr = VirtualAlloc(NULL, reserveSize, MEM_RESERVE, PAGE_NOACCESS);
    if (!baseAddr) {
        printf("Memory reserve failed\n");
        return 1;
    }
    printf("256 pages reserved at address: %p\n", baseAddr);
    system("pause"); // Step 1: check VMMap and RAMMap

    // 3. Commit physical memory for the second half (128 pages)
    LPVOID commitAddr = (LPBYTE)baseAddr + (128 * pageSize);
    LPVOID committed = VirtualAlloc(commitAddr, 128 * pageSize, MEM_COMMIT, PAGE_READWRITE);
    if (!committed) {
        printf("Memory commit failed\n");
        VirtualFree(baseAddr, 0, MEM_RELEASE);
        return 1;
    }
    printf("Physical memory committed for second half at: %p\n", committed);
    system("pause"); // Step 2: check VMMap and RAMMap

    // 4. Fill the second half with numbers
    int* arr = (int*)committed;
    SIZE_T count = (128 * pageSize) / sizeof(int);
    for (SIZE_T i = 0; i < count; i++) {
        arr[i] = (int)i;
    }
    printf("Filled %llu integers\n", (unsigned long long)count);
    system("pause"); // Step 3: check VMMap and RAMMap

    // 5. Change memory protection to read-only
    DWORD oldProtect;
    if (!VirtualProtect(committed, 128 * pageSize, PAGE_READONLY, &oldProtect)) {
        printf("Failed to change memory protection\n");
    }
    else {
        printf("Memory protection changed to READ ONLY\n");
    }
    system("pause"); // Step 4: check VMMap
    if (!VirtualFree(committed, 128 * pageSize, MEM_DECOMMIT)) {
        printf("Failed to free physical memory\n");
    }
    else {
        printf("Physical memory decommitted\n");
    }
    system("pause"); // Step 5: check VMMap and RAMMap

    // 7. Release entire virtual memory region (256 pages)
    if (!VirtualFree(baseAddr, 0, MEM_RELEASE)) {
        printf("Failed to release virtual memory\n");
    }
    else {
        printf("Virtual memory released\n");
    }
    system("pause"); // Step 6: check VMMap
    return 0;
}
