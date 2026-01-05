#!/bin/bash

echo "Compiling all Lab-03 projects..."

echo -e "\n============ Level 4-5 ============"
cd level_4-5

echo "Compiling Lab-03x..."
g++ -o Lab-03x Lab-03x.cpp

echo "Compiling Lab-03a..."
g++ -o Lab-03a Lab-03a.cpp

echo "Compiling Lab-03b..."
g++ -o Lab-03b Lab-03b.cpp

echo "Compiling Lab-03c..."
g++ -o Lab-03c Lab-03c.cpp

cd ..

echo -e "\n============ Level 7-8 ============"
cd level_7-8

echo "Compiling Lab-03d-client..."
g++ -o Lab-03d-client Lab-03d-client.cpp

echo "Compiling Lab-03d-server..."
g++ -o Lab-03d-server Lab-03d-server.cpp

cd ..

echo -e "\nMaking executables executable..."
chmod +x level_4-5/Lab-03x
chmod +x level_4-5/Lab-03a
chmod +x level_4-5/Lab-03b
chmod +x level_4-5/Lab-03c
chmod +x level_7-8/Lab-03d-client
chmod +x level_7-8/Lab-03d-server

echo -e "\nAll projects compiled successfully!"