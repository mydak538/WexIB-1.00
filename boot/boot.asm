[BITS 16]
[ORG 0x7C00]

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

    ; Очистка экрана
    mov ax, 0x0003
    int 0x10

    mov si, msg_loading
    call print_string

    ; Загружаем BIOS код (64 сектора = 32KB)
    mov ax, 0x0240      ; 64 сектора
    mov cx, 0x0002      ; цилиндр 0, сектор 2
    mov dh, 0x00        ; головка 0
    mov dl, 0x80        ; диск
    mov bx, 0x7E00      ; загружаем по адресу 0x7E00
    int 0x13
    jc disk_error

    ; Переходим в защищенный режим
    call switch_to_protected_mode

; Включаем A20 линию
enable_a20:
    in al, 0x92
    or al, 2
    out 0x92, al
    ret

; Загружаем GDT
load_gdt:
    lgdt [gdt_descriptor]
    ret

; Переключаемся в защищенный режим
switch_to_protected_mode:
    cli
    call enable_a20
    call load_gdt
    
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    
    jmp CODE_SEG:init_pm

[BITS 32]
init_pm:
    ; Настраиваем сегменты
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000
    
    ; Вызываем главную C функцию BIOS
    call 0x7E00
    
    jmp $

; GDT
gdt_start:
    gdt_null:
        dd 0x0
        dd 0x0
    gdt_code:
        dw 0xFFFF
        dw 0x0
        db 0x0
        db 10011010b
        db 11001111b
        db 0x0
    gdt_data:
        dw 0xFFFF
        dw 0x0
        db 0x0
        db 10010010b
        db 11001111b
        db 0x0
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

; Функции реального режима
print_string:
    mov ah, 0x0E
.loop:
    lodsb
    test al, al
    jz .done
    int 0x10
    jmp .loop
.done:
    ret

disk_error:
    mov si, msg_error
    call print_string
    jmp $

; Сообщения
msg_loading db "Loading BIOS...", 0x0D, 0x0A, 0
msg_error db "Disk Error! The main BIOS firmware is damaged or not found! Try to reboot if it doesn't work then reflash to an older BIOS firmware current version 4.51 Firmware can be downloaded here https://github.com/mydak538/WexIB Instructions for flashing this device are available here: https://github.com/mydak538/WexIB/blob/main/flashing.md", 0

times 510-($-$$) db 0
dw 0xAA55

