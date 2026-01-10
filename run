#!/bin/bash
echo "=== Custom BIOS for Linux ==="

if [ $? -eq 0 ]; then
    echo "Starting QEMU..."
    qemu-system-x86_64 -drive format=raw,file=bin/bios.img -net none
else
    echo "Build failed!"
    read -p "Press enter to continue..."
fi
