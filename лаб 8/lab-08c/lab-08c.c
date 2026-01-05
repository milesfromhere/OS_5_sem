#include <windows.h>
#include <stdio.h>
#include <locale.h>

// Вспомогательная функция для форматирования размера
static void fmtBytes(SIZE_T bytes, char* buf, size_t bufSize) {
    double kib = bytes / 1024.0;
    double mib = kib / 1024.0;
    if (mib >= 1.0)
        snprintf(buf, bufSize, "%.2f MiB", mib);
    else
        snprintf(buf, bufSize, "%.2f KiB", kib);
}

// Функция вывода детальной информации о куче
void HeapInfo(HANDLE heap) {
    PROCESS_HEAP_ENTRY entry;
    entry.lpData = NULL;

    printf("\n--- Детальный отчет о куче ---\n");
    printf("%-20s | %-12s | %-15s\n", "Адрес начала", "Размер", "Тип области");
    printf("----------------------------------------------------------\n");

    while (HeapWalk(heap, &entry)) {
        char sizeBuf[32];
        fmtBytes(entry.cbData, sizeBuf, sizeof(sizeBuf));

        const char* typeStr = "Unknown";
        if (entry.wFlags & PROCESS_HEAP_ENTRY_BUSY) {
            typeStr = "BUSY (Занято)";
        }
        else if (entry.wFlags & PROCESS_HEAP_REGION) {
            typeStr = "REGION (Начало)";
        }
        else if (entry.wFlags & PROCESS_HEAP_UNCOMMITTED_RANGE) {
            typeStr = "UNCOMMITTED";
        }
        else {
            typeStr = "FREE (Свободно)";
        }

        printf("%p | %-12s | %-15s\n", entry.lpData, sizeBuf, typeStr);
    }

    DWORD err = GetLastError();
    if (err != ERROR_NO_MORE_ITEMS) {
        printf("Ошибка HeapWalk: %lu\n", err);
    }
    printf("----------------------------------------------------------\n\n");
}

int main() {
    setlocale(LC_ALL, "ru");

    // Параметры задания (можно менять для теста со звездочкой)
    const SIZE_T initialSize = 1024 * 1024;     // 1 МиБ
    const SIZE_T maxSize = 8 * 1024 * 1024;        // 8 МиБ
    const SIZE_T blockSize = 1024 * 1024;        // 512 КиБ
    const int blockCount = 10;                  // 10 блоков

    // 1. Создание кучи
    HANDLE heap = HeapCreate(0, initialSize, maxSize);
    if (!heap) {
        printf("Ошибка HeapCreate: %lu\n", GetLastError());
        return 1;
    }
    printf("Этап 1: Куча создана. Начальный размер: 1 МиБ, Макс: 8 МиБ.\n");
    HeapInfo(heap); system("pause & cls");

    // 2. Выделение блоков
    void* blocks[20]; // Запас для экспериментов
    for (int i = 0; i < blockCount; ++i) {
        blocks[i] = HeapAlloc(heap, HEAP_ZERO_MEMORY, blockSize);
        if (!blocks[i]) {
            printf("Ошибка HeapAlloc на итерации %d: %lu\n", i, GetLastError());
            break;
        }
        printf("Этап 2: Выделен блок %d (%p)\n", i + 1, blocks[i]);
        HeapInfo(heap); system("pause & cls");
    }

    // 3. Заполнение данными (без HeapInfo по заданию)
    for (int i = 0; i < blockCount; ++i) {
        if (blocks[i]) {
            int* arr = (int*)blocks[i];
            for (SIZE_T j = 0; j < (blockSize / sizeof(int)); ++j) {
                arr[j] = (int)j;
            }
        }
    }
    printf("Этап 3: Блоки заполнены данными.\n");
    system("pause & cls");

    // 4. Освобождение
    for (int i = 0; i < blockCount; ++i) {
        if (blocks[i]) {
            HeapFree(heap, 0, blocks[i]);
        }
    }
    printf("Этап 4: Память освобождена.\n");
    HeapInfo(heap); system("pause & cls");

    // 5. Уничтожение
    HeapDestroy(heap);
    printf("Этап 5: Куча уничтожена.\n");

    return 0;
}
