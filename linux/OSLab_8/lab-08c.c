#include <stdlib.h>
#include <stdio.h>
#include <locale.h>
#include <unistd.h>

// Функция для печати размера в КиБ/МиБ
void fmtBytes(size_t bytes, char* buf, size_t bufSize) {
    double kib = bytes / 1024.0;
    double mib = kib / 1024.0;
    if (mib >= 1.0)
        snprintf(buf, bufSize, "%.2f MiB", mib);
    else
        snprintf(buf, bufSize, "%.2f KiB", kib);
}
int main() {
    setlocale(LC_ALL, "ru");

    // Размер страницы (для справки)
    long pageSize = sysconf(_SC_PAGESIZE);
    printf("Размер страницы: %ld байт\n", pageSize);

    // Этап 1: создаём "кучу" (в Linux это просто malloc/free)
    printf("Этап 1: старт программы, наблюдаем pmap.\n");
    getchar();

    // Этап 2: выделяем 10 блоков по 512 КиБ
    const size_t blockSize = 512ull * 1024; // 512 КиБ
    const int blockCount = 10;
    void* blocks[blockCount];

    for (int i = 0; i < blockCount; ++i) {
        void* p = malloc(blockSize);
        if (!p) {
            char buf[64];
            fmtBytes(blockSize, buf, sizeof(buf));
            printf("malloc: не удалось выделить блок #%d размером %s\n", i, buf);
            break;
        }
        blocks[i] = p;
        char buf[64];
        fmtBytes(blockSize, buf, sizeof(buf));
        printf("Выделен блок #%d addr=%p size=%s\n", i, p, buf);
        getchar(); // смотри pmap после каждой итерации
    }

    // Этап 3: заполняем каждый блок числами
    for (int i = 0; i < blockCount; ++i) {
        if (blocks[i]) {

            int* arr = (int*)blocks[i];
            size_t count = blockSize / sizeof(int);
            for (size_t j = 0; j < count; ++j) {
                arr[j] = (int)j;
            }
        }
    }
    printf("Этап 3: заполнение всех блоков завершено (%d блоков).\n", blockCount);
    getchar(); // смотри pmap

    // Этап 4: освобождаем все блоки
    for (int i = 0; i < blockCount; ++i) {
        if (blocks[i]) {
            free(blocks[i]);
        }
    }
    printf("Этап 4: освобождение всех блоков завершено.\n");
    getchar(); // смотри pmap
    // Этап 5: завершение программы
    printf("Этап 5: программа завершает работу, куча уничтожена.\n");
    getchar();
    return 0;
}
