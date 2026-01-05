#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>

void PrintInfo(const char* fileName) {
    struct stat st;
    if (stat(fileName, &st) == -1) {
        perror("Ошибка: не удалось получить информацию о файле");
        return;
    }

    printf("Имя файла: %s\n", fileName);
    printf("Размер: %lld Б (%.2f КиБ, %.2f МиБ)\n",
           (long long)st.st_size,
           (double)st.st_size / 1024.0,
           (double)st.st_size / (1024.0 * 1024.0));

    if (S_ISDIR(st.st_mode))
        printf("Тип: каталог\n");
    else if (S_ISREG(st.st_mode))
        printf("Тип: обычный файл\n");
    else
        printf("Тип: другой объект\n");

    char buf[64];
    struct tm *tm_info;

    tm_info = localtime(&st.st_ctime);
    strftime(buf, sizeof(buf), "%d.%m.%Y %H:%M", tm_info);
    printf("Время создания: %s\n", buf);

    tm_info = localtime(&st.st_atime);
    strftime(buf, sizeof(buf), "%d.%m.%Y %H:%M", tm_info);
    printf("Последний доступ: %s\n", buf);

    tm_info = localtime(&st.st_mtime);
    strftime(buf, sizeof(buf), "%d.%m.%Y %H:%M", tm_info);
    printf("Последнее изменение: %s\n", buf);
}

void PrintText(const char* fileName) {
    int fd = open(fileName, O_RDONLY);
    if (fd == -1) {
        perror("Ошибка: не удалось открыть файл");
        return;
    }

    char buffer[512];
    ssize_t bytesRead;
    while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0) {
        fwrite(buffer, 1, bytesRead, stdout);
    }

    close(fd);
}

int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "ru_RU.UTF-8");

    if (argc < 2) {
        printf("Использование: %s <имя_файла>\n", argv[0]);
        return 1;
    }

    PrintInfo(argv[1]);
    printf("\n--- Содержимое файла ---\n");
    PrintText(argv[1]);

    return 0;
}
