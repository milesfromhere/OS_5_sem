#!/bin/bash

# Скрипт для подсчета исполняемых и неисполняемых команд в PATH

count_executable=0
count_non_executable=0

# Разбиваем PATH на отдельные директории
IFS=':' read -ra directories <<< "$PATH"

# Проходим по каждой директории в PATH
for dir in "${directories[@]}"; do
    # Проверяем существование директории
    if [[ -d "$dir" ]]; then
        # Проходим по всем файлам в директории
        for file in "$dir"/*; do
            # Проверяем, является ли обычным файлом
            if [[ -f "$file" ]]; then
                # Проверяем, является ли исполняемым
                if [[ -x "$file" ]]; then
                    ((count_executable++))
                else
                    ((count_non_executable++))
                fi
            fi
        done
    fi
done

echo "Исполняемые команды: $count_executable"
echo "Неисполняемые команды: $count_non_executable"
echo "Всего команд: $((count_executable + count_non_executable))"