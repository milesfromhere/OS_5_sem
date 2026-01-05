#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <locale.h>

#define MAX_LINE_LEN 1024
int g_fd = -1;
char *g_fileBuf = NULL;
size_t g_fileSize = 0;
char **g_lines = NULL;
size_t g_lineCount = 0;
static char* dup_n(const char* src, size_t n) {
    char* s = (char*)malloc(n + 1);
    if (!s) return NULL;
    memcpy(s, src, n);
    s[n] = '\0';
    return s;
}
void FreeGlobals() {
    if (g_lines) {
        for (size_t i = 0; i < g_lineCount; i++) {
            free(g_lines[i]);
        }
        free(g_lines);
        g_lines = NULL;
    }
    if (g_fileBuf) { free(g_fileBuf); g_fileBuf = NULL; }
    g_fileSize = 0;
    g_lineCount = 0;
}

int EnsureOpen() {
    if (g_fd == -1) {
        printf("Ошибка: файл не открыт.\n");
        return 0;
    }
    return 1;
}

int LoadFileToBuffer() {

    struct stat st;
    if (fstat(g_fd, &st) == -1) {
        perror("Ошибка получения размера файла");
        return 0;
    }
    g_fileSize = st.st_size;
    g_fileBuf = (char*)malloc(g_fileSize + 1);
    if (!g_fileBuf) {
        printf("Ошибка выделения памяти.\n");
        return 0;
    }
    if (g_fileSize == 0) { g_fileBuf[0] = '\0'; return 1; }

    if (lseek(g_fd, 0, SEEK_SET) < 0) {
        perror("Ошибка позиционирования файла");
        return 0;
    }
    ssize_t r = read(g_fd, g_fileBuf, g_fileSize);
    if (r < 0 || (size_t)r != g_fileSize) {
        perror("Ошибка чтения файла");
        return 0;
    }
    g_fileBuf[g_fileSize] = '\0';
    return 1;
}

int ParseLines() {
    
    size_t count = 0;
    for (size_t i = 0; i < g_fileSize; i++) {
        if (g_fileBuf[i] == '\n') count++;
    }
    if (g_fileSize > 0 && count == 0) count = 1;

    g_lines = (char**)malloc(sizeof(char*) * (count ? count : 1));
    if (!g_lines) { printf("Ошибка выделения памяти.\n"); return 0; }

    size_t idx = 0;
    char* start = g_fileBuf;
    for (size_t i = 0; i < g_fileSize; i++) {
        if (g_fileBuf[i] == '\n') {
            // строка от start до i-1, возможно CRLF — убираем '\r'
            size_t len = (size_t)(g_fileBuf + i - start);
            if (len > 0 && start[len - 1] == '\r') len--;
            char* s = dup_n(start, len);
            if (!s) { printf("Ошибка выделения памяти строки.\n"); return 0; }
            g_lines[idx++] = s;
            start = g_fileBuf + i + 1;

        }
    }
    if ((size_t)(start - g_fileBuf) < g_fileSize) {
        size_t len = (size_t)(g_fileBuf + g_fileSize - start);
        if (len > 0) {
            if (len > 0 && start[len - 1] == '\r') len--;
            char* s = dup_n(start, len);
            if (!s) { printf("Ошибка выделения памяти строки.\n"); return 0; }
            g_lines[idx++] = s;
        }
    }

    g_lineCount = idx;
    return 1;
}

int RefreshFromDisk() {
    FreeGlobals();
    if (!LoadFileToBuffer()) return 0;
    return ParseLines();
}

int SaveBufferToFile() {
    if (!EnsureOpen()) return 0;
    size_t total = 0;
    for (size_t i = 0; i < g_lineCount; i++) total += strlen(g_lines[i]) + 1;

    char *out = (char*)malloc(total + 1);
    if (!out) { printf("Ошибка выделения памяти.\n"); return 0; }

    char *w = out;
    for (size_t i = 0; i < g_lineCount; i++) {
        size_t len = strlen(g_lines[i]);
        memcpy(w, g_lines[i], len);
        w += len;
        *w++ = '\n';
    }
    *w = '\0';

    if (lseek(g_fd, 0, SEEK_SET) < 0) {
        perror("Ошибка позиционирования файла");
        free(out);
        return 0;
    }
    ssize_t wr = write(g_fd, out, (size_t)(w - out));
    if (wr < 0 || (size_t)wr != (size_t)(w - out)) {
        perror("Ошибка записи");
        free(out);

        return 0;
    }
    if (ftruncate(g_fd, (off_t)(w - out)) < 0) {
        perror("Ошибка обрезки файла");
        free(out);
        return 0;
    }
    free(out);
    return RefreshFromDisk();
}

int OpenFile(const char *path) {
    if (g_fd != -1) {
        printf("Ошибка: файл уже открыт.\n");
        return 0;
    }
    g_fd = open(path, O_RDWR);
    if (g_fd == -1) {
        perror("Ошибка открытия файла");
        return 0;
    }
    if (!LoadFileToBuffer() || !ParseLines()) {
        close(g_fd); g_fd = -1; FreeGlobals();
        return 0;
    }
    printf("Файл открыт. Строк: %lu, Размер: %lu байт.\n",
           (unsigned long)g_lineCount, (unsigned long)g_fileSize);
    return 1;
}

int CloseFile() {
    if (g_fd == -1) {
        printf("Ошибка: файл не открыт.\n");
        return 0;
    }
    FreeGlobals();
    close(g_fd);
    g_fd = -1;
    printf("Файл закрыт.\n");
    return 1;
}

int AddRow(const char *row, int pos) {
    if (!EnsureOpen()) return 0;
    if (!row) { printf("Ошибка: строка NULL.\n"); return 0; }

    size_t insertIdx = 0;
    if (pos == -1) insertIdx = g_lineCount;
    else if (pos == 0) insertIdx = 0;

    else if (pos > 0 && (size_t)pos <= g_lineCount + 1) insertIdx = (size_t)pos - 1;
    else { printf("Ошибка: недопустимая позиция.\n"); return 0; }

    char **newLines = (char**)malloc(sizeof(char*) * (g_lineCount + 1));
    if (!newLines) { printf("Ошибка выделения памяти.\n"); return 0; }

    for (size_t i = 0; i < insertIdx; i++) newLines[i] = g_lines[i];

    char *copy = strdup(row);
    if (!copy) { free(newLines); printf("Ошибка выделения памяти строки.\n"); return 0; }
    newLines[insertIdx] = copy;

    for (size_t i = insertIdx; i < g_lineCount; i++) newLines[i + 1] = g_lines[i];

    free(g_lines);
    g_lines = newLines;
    g_lineCount++;

    if (!SaveBufferToFile()) return 0;
    printf("Строка добавлена на позицию %d.\n", pos);
    return 1;
}

int RemRow(int pos) {
    if (!EnsureOpen()) return 0;
    if (g_lineCount == 0) { printf("Ошибка: файл пуст.\n"); return 0; }

    size_t idx;
    if (pos == -1) idx = g_lineCount - 1;
    else if (pos == 0) idx = 0;
    else if (pos > 0 && (size_t)pos <= g_lineCount) idx = (size_t)pos - 1;
    else { printf("Ошибка: недопустимая позиция.\n"); return 0; }

    char **newLines = NULL;
    if (g_lineCount > 1) {
        newLines = (char**)malloc(sizeof(char*) * (g_lineCount - 1));
        if (!newLines) { printf("Ошибка выделения памяти.\n"); return 0; }
    }

    free(g_lines[idx]);

    for (size_t i = 0; i < idx; i++) if (newLines) newLines[i] = g_lines[i];
    for (size_t i = idx + 1; i < g_lineCount; i++) if (newLines) newLines[i - 1] = g_lines[i];

    free(g_lines);
    g_lines = newLines;
    g_lineCount--;

    if (!SaveBufferToFile()) return 0;
    printf("Строка удалена с позиции %d.\n", pos);
    return 1;
}

int PrintRow(int pos) {
    if (!EnsureOpen()) return 0;
    if (g_lineCount == 0) { printf("Ошибка: файл пуст.\n"); return 0; }

    size_t idx;
    if (pos == -1) idx = g_lineCount - 1;
    else if (pos == 0) idx = 0;
    else if (pos > 0 && (size_t)pos <= g_lineCount) idx = (size_t)pos - 1;
    else { printf("Ошибка: недопустимая позиция.\n"); return 0; }

    printf("%lu: %s\n", (unsigned long)(idx + 1), g_lines[idx]);
    return 1;
}

int PrintRows() {
    if (!EnsureOpen()) return 0;
    for (size_t i = 0; i < g_lineCount; i++) {
        printf("%lu. %s\n", (unsigned long)(i + 1), g_lines[i]);
    }
    return 1;
}

int main() {
    setlocale(LC_ALL, "ru_RU.UTF-8");
    while (1) {
        printf("\nМеню:\n");
        printf("1. Открыть файл\n");
        printf("2. Вставить строку\n");
        printf("3. Удалить строку\n");
        printf("4. Вывести строку\n");
        printf("5. Вывести файл\n");
        printf("6. Закрыть файл\n");
        printf("0. Выход\n> ");

        int cmd;
        if (scanf("%d", &cmd) != 1) { printf("Ошибка ввода.\n"); break; }
        getchar(); // очистка \n

        if (cmd == 0) {
            if (g_fd != -1) CloseFile();
            break;
        }

        switch (cmd) {
        case 1: {
            char path[256];
            printf("Введите путь к файлу: ");
            if (!fgets(path, sizeof(path), stdin)) { printf("Ошибка ввода.\n"); break; }
            path[strcspn(path, "\n")] = '\0';
            OpenFile(path);
            break;
        }
        case 2: {
            char row[MAX_LINE_LEN];
            int pos;
            printf("Позиция вставки (0 — начало, -1 — конец, >0 — номер): ");
            if (scanf("%d", &pos) != 1) { printf("Ошибка ввода.\n"); break; }
            getchar();
            printf("Введите строку: ");
            if (!fgets(row, sizeof(row), stdin)) { printf("Ошибка ввода.\n"); break; }
            row[strcspn(row, "\n")] = '\0';
            AddRow(row, pos);
            break;
        }
        case 3: {
            int pos;
            printf("Позиция удаления (0 — первая, -1 — последняя, >0 — номер): ");
            if (scanf("%d", &pos) != 1) { printf("Ошибка ввода.\n"); break; }
            getchar();
            RemRow(pos);
            break;
        }
        case 4: {
            int pos;
            printf("Позиция вывода (0 — первая, -1 — последняя, >0 — номер): ");
            if (scanf("%d", &pos) != 1) { printf("Ошибка ввода.\n"); break; }
            getchar();
            PrintRow(pos);

            break;
        }
        case 5:
            PrintRows();
            break;
        case 6:
            CloseFile();
            break;
        default:
            printf("Неизвестная команда.\n");
        }
    }
    return 0;
}
