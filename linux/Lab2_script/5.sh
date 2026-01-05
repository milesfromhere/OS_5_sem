#!/bin/bash

mode=$1
file=$2

if [[ -z "$mode" || -z "$file" ]]; then
    echo "Использование: bash_05.sh создать|удалить имя_файла"
    exit 1
fi

if [[ "$mode" == "создать" ]]; then
    if [[ -e "$file" ]]; then
        echo "Файл уже существует"
    else
        touch "$file"
        echo "Файл создан"
    fi
elif [[ "$mode" == "удалить" ]]; then
    if [[ -e "$file" ]]; then
        rm "$file"
        echo "Файл удален"
    else
        echo "Файл не найден"
    fi
else
    echo "Неизвестная команда: $mode"
    echo "Использование: bash_05.sh создать|удалить имя_файла"
    exit 1
fi
