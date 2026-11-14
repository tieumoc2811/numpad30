#!/bin/sh
# Chay: sh test/run_tests.sh   (can gcc)
set -e
cd "$(dirname "$0")"
mkdir -p build
gcc -std=gnu11 -Wall -Wextra -Wno-unused-function -O1 -fsanitize=address,undefined -I../firmware -o build/test_core test_core.c
./build/test_core
gcc -std=gnu11 -Wall -Wextra -Wno-unused-function -O1 -fsanitize=address,undefined -I../firmware -o build/test_ext test_ext.c
./build/test_ext
# End-to-end: nhung chinh firmware/main.c vao khung gia (stub/ thay Pico SDK + TinyUSB), bam phim vat ly, giai ma bao cao HID
gcc -std=gnu11 -Wall -Wno-unused-function -Wno-unused-variable -O1 -fsanitize=address,undefined -Istub -I../firmware -o build/fw_sim fw_sim.c
./build/fw_sim
# Web Studio (can node + playwright; bo qua neu khong co)
if command -v node >/dev/null 2>&1 && node -e "require('playwright')" >/dev/null 2>&1; then node web_test.js; else echo "web: bo qua (khong co node/playwright)"; fi
