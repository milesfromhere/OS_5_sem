#define _XOPEN_SOURCE 700
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <dirent.h>
#include <limits.h>

#ifndef BUF_LEN
#define BUF_LEN (1024 * (sizeof(struct inotify_event) + NAME_MAX + 1))
#endif

static void PrintDirectoryContents(const char* path) {
    DIR* d = opendir(path);
    if (!d) {
        fprintf(stderr, "Ошибка: не удалось прочитать каталог %s: %s\n", path, strerror(errno));
        return;
    }
    printf("Содержимое каталога %s:\n", path);
    struct dirent* de;
    while ((de = readdir(d)) != NULL) {
        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) continue;
        if (de->d_type == DT_DIR) printf("[DIR]  %s\n", de->d_name);
        else if (de->d_type == DT_REG) printf("[FILE] %s\n", de->d_name);
        else printf("[OTHER] %s\n", de->d_name);
    }
    closedir(d);
}

static void print_mask(uint32_t mask, const char* name) {
    // Игнорируем события без имени
    if (name == NULL || strlen(name) == 0) {
        return;
    }
    
    // Показываем тип объекта
    if (mask & IN_ISDIR) {
        printf("[КАТАЛОГ] ");
    } else {
        printf("[ФАЙЛ] ");
    }
    
    // ОСНОВНЫЕ СОБЫТИЯ:
    if (mask & IN_CREATE) {
        printf("СОЗДАН: %s\n", name);
    }
    else if (mask & IN_DELETE) {
        printf("УДАЛЁН: %s\n", name);
    }
    else if (mask & IN_MODIFY) {
        printf("ИЗМЕНЁН: %s\n", name);
    }
    else if (mask & IN_MOVED_FROM) {
        printf("ПЕРЕИМЕНОВАН ИЗ: %s\n", name);
    }
    else if (mask & IN_MOVED_TO) {
        printf("ПЕРЕИМЕНОВАН В: %s\n", name);
    }
    else if (mask & IN_ATTRIB) {
        printf("ИЗМЕНЕНЫ АТРИБУТЫ: %s\n", name);
    }
    // СОБЫТИЯ ОТКРЫТИЯ/ЗАКРЫТИЯ (добавили):
    else if (mask & IN_OPEN) {
        printf("ОТКРЫТ: %s\n", name);
    }
    else if (mask & IN_ACCESS) {
        printf("ПРОЧИТАН: %s\n", name);
    }
    else if (mask & IN_CLOSE_WRITE) {
        printf("ЗАКРЫТ ПОСЛЕ ЗАПИСИ: %s\n", name);
    }
    else if (mask & IN_CLOSE_NOWRITE) {
        printf("ЗАКРЫТ БЕЗ ЗАПИСИ: %s\n", name);
    }
}

static int is_directory(const char* path) {
    struct stat st;
    if (stat(path, &st) != 0) return 0;
    return S_ISDIR(st.st_mode);
}

static void WatchDirectory(const char* path, int recursive) {
    int fd = inotify_init1(IN_NONBLOCK);
    if (fd == -1) {
        fprintf(stderr, "Ошибка inotify_init1: %s\n", strerror(errno));
        return;
    }

    // Добавили события открытия/закрытия файлов:
    uint32_t mask = IN_CREATE | IN_DELETE | IN_MODIFY | 
                    IN_MOVED_FROM | IN_MOVED_TO | IN_ATTRIB |
                    IN_OPEN | IN_ACCESS | IN_CLOSE_WRITE | IN_CLOSE_NOWRITE;

    int wd = inotify_add_watch(fd, path, mask);
    if (wd == -1) {
        fprintf(stderr, "Ошибка inotify_add_watch для %s: %s\n", path, strerror(errno));
        close(fd);
        return;
    }

    if (recursive) {
        DIR* d = opendir(path);
        if (d) {
            struct dirent* de;
            char subpath[PATH_MAX];
            while ((de = readdir(d)) != NULL) {
                if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) continue;
                snprintf(subpath, sizeof(subpath), "%s/%s", path, de->d_name);
                if (is_directory(subpath)) {
                    int swd = inotify_add_watch(fd, subpath, mask);
                    if (swd == -1) {
                        fprintf(stderr, "Ошибка add_watch для %s: %s\n", subpath, strerror(errno));
                    }
                }
            }
            closedir(d);
        }
    }

    printf("\nОтслеживание изменений в каталоге %s...\n", path);
    printf("События: создание, удаление, изменение, переименование, открытие, закрытие файлов\n");
    printf("(Нажмите Ctrl+C для выхода)\n\n");

    char buf[BUF_LEN];
    for (;;) {
        ssize_t len = read(fd, buf, sizeof(buf));
        if (len == -1) {
            if (errno == EAGAIN) {
                sleep(1);
                continue;
            } else {
                fprintf(stderr, "Ошибка чтения событий: %s\n", strerror(errno));
                break;
            }
        }
        if (len == 0) continue;

        for (char *ptr = buf; ptr < buf + len; ) {
            struct inotify_event *ev = (struct inotify_event *)ptr;
            
            const char* name = NULL;
            if (ev->len > 0 && ev->name[0] != '\0') {
                name = ev->name;
            }
            
            if (name != NULL) {
                print_mask(ev->mask, name);
                
                if (recursive && (ev->mask & IN_CREATE) && (ev->mask & IN_ISDIR)) {
                    char newdir[PATH_MAX];
                    snprintf(newdir, sizeof(newdir), "%s/%s", path, name);
                    int swd = inotify_add_watch(fd, newdir, mask);
                    if (swd != -1) {
                        printf("[INFO] Добавлено наблюдение за новым каталогом: %s\n", name);
                    }
                }
            }
            
            ptr += sizeof(struct inotify_event) + ev->len;
        }
    }

    close(fd);
}

int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "ru_RU.UTF-8");

    if (argc < 2) {
        printf("Использование: %s <путь_к_каталогу> [--recursive]\n", argv[0]);
        return 1;
    }

    const char* path = argv[1];
    int recursive = (argc >= 3 && strcmp(argv[2], "--recursive") == 0);

    struct stat st;
    if (stat(path, &st) != 0 || !S_ISDIR(st.st_mode)) {
        fprintf(stderr, "Ошибка: каталог %s не существует.\n", path);
        return 1;
    }

    PrintDirectoryContents(path);
    WatchDirectory(path, recursive);
    return 0;
}