#!/bin/bash
echo "Имя скрипта: $(basename "$0")"
echo "Полный путь: $(realpath "$0")"
echo "Дата последней конфигурации: $(stat -c %y "$0")"

