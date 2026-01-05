#!/bin/bash

echo "Running Lab-03 tests..."

# Установка глобальной переменной окружения
export ITER_NUM=7

echo -e "\n============ Testing Lab-03x ============"
echo "Testing with command line argument:"
./Level-4-5/Lab-03x 3

echo -e "\nTesting with environment variable:"
./Level-4-5/Lab-03x

echo -e "\n============ Testing Lab-03c ============"
./Level-4-5/Lab-03c | head -20

echo -e "\n============ Testing Lab-03a ============"
./Level-4-5/Lab-03a

echo -e "\n============ Testing Lab-03b ============"
./Level-4-5/Lab-03b

echo -e "\nTests completed!"