#include <stdio.h>
#include <locale.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>

int main() {
    setlocale(LC_ALL, "ru");

    // 1. Получаем размер страницы
    long pageSize = sysconf(_SC_PAGESIZE);
    printf("Размер страницы: %ld байт\n", pageSize);

    // 2. Резервируем 256 страниц (PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS)
    size_t reserveSize = 256 * pageSize;
    void* baseAddr = mmap(NULL, reserveSize, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (baseAddr == MAP_FAILED) {
        printf("Ошибка резервирования памяти\n");
        return 1;

    }
    printf("Резервировано 256 страниц по адресу: %p\n", baseAddr);
    getchar(); // Этап 1 — смотри pmap

    // 3. Выделяем физическую память для второй половины (128 страниц)
    void* commitAddr = (char*)baseAddr + (128 * pageSize);
    if (mprotect(commitAddr, 128 * pageSize, PROT_READ | PROT_WRITE) != 0) {
        printf("Ошибка коммита памяти\n");
        munmap(baseAddr, reserveSize);
        return 1;
    }
    printf("Выделена физическая память для второй половины: %p\n", commitAddr);
    getchar(); // Этап 2 — смотри pmap

    // 4. Заполняем вторую половину числами
    int* arr = (int*)commitAddr;
    size_t count = (128 * pageSize) / sizeof(int);
    for (size_t i = 0; i < count; ++i) {
        arr[i] = (int)i;
    }
    printf("Заполнено %zu целых чисел.\n", count);
    getchar(); // Этап 3 — смотри pmap

    // 5. Меняем защиту на "только чтение"
    if (mprotect(commitAddr, 128 * pageSize, PROT_READ) != 0) {
        printf("Ошибка изменения защиты\n");
    } else {
        printf("Защита изменена на READONLY.\n");
    }
    getchar(); // Этап 4 — смотри pmap

    // 6. Освобождаем 128 страниц (вторую половину)
    if (munmap(commitAddr, 128 * pageSize) != 0) {
        printf("Ошибка освобождения физической памяти\n");
    } else {
        printf("Физическая память освобождена.\n");
    }
    getchar(); // Этап 5 — смотри pmap

    // 7. Освобождаем оставшиеся 128 страниц
    if (munmap(baseAddr, 128 * pageSize) != 0) {
        printf("Ошибка освобождения виртуальной памяти\n");
    } else {
        printf("Виртуальная память освобождена.\n");
    }
    getchar(); // Этап 6 — смотри pmap

    return 0;
}
