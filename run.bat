@echo off
echo === Custom BIOS for Windows ===

if %errorlevel% equ 0 (
    echo Starting QEMU...
    "C:\Program Files\qemu\qemu-system-x86_64.exe" -drive format=raw,file=bin\bios.img -net none
) else (
    echo Build failed!
    pause
)