#include "cpu.h"
#include "console.h"
#include <stdint.h>

// Внешние функции
extern void print_string(const char* str, uint8_t x, uint8_t y, uint8_t color);

// Наши реализации строковых функций
static void my_strcpy(char* dest, const char* src) {
    while (*src) {
        *dest++ = *src++;
    }
    *dest = '\0';
}

static int my_strlen(const char* str) {
    int len = 0;
    while (*str++) len++;
    return len;
}

// Проверка наличия CPUID (для 486 и выше)
static uint8_t cpu_has_cpuid(void) {
    uint32_t flags;
    
    // Сохраняем оригинальные EFLAGS
    __asm__ volatile (
        ".code32\n"
        "pushfl\n"
        "popl %0\n"
        : "=r" (flags)
        :
        : "cc"
    );
    
    // Пытаемся изменить бит ID (21-й бит)
    uint32_t flags_toggled = flags ^ (1 << 21);
    
    __asm__ volatile (
        ".code32\n"
        "pushl %0\n"
        "popfl\n"
        :
        : "r" (flags_toggled)
        : "cc"
    );
    
    __asm__ volatile (
        ".code32\n"
        "pushfl\n"
        "popl %0\n"
        : "=r" (flags_toggled)
        :
        : "cc"
    );
    
    // Восстанавливаем оригинальные флаги
    __asm__ volatile (
        ".code32\n"
        "pushl %0\n"
        "popfl\n"
        :
        : "r" (flags)
        : "cc"
    );
    
    // Если бит можно изменить, CPUID поддерживается
    return ((flags ^ flags_toggled) & (1 << 21)) != 0;
}

// Проверка 386/486 через флаг AC (бит 18)
static uint8_t cpu_has_ac_flag(void) {
    uint32_t flags;
    
    __asm__ volatile (
        ".code32\n"
        "pushfl\n"
        "popl %0\n"
        : "=r" (flags)
        :
        : "cc"
    );
    
    // Пытаемся переключить бит AC
    uint32_t flags_toggled = flags ^ (1 << 18);
    
    __asm__ volatile (
        ".code32\n"
        "pushl %0\n"
        "popfl\n"
        :
        : "r" (flags_toggled)
        : "cc"
    );
    
    __asm__ volatile (
        ".code32\n"
        "pushfl\n"
        "popl %0\n"
        : "=r" (flags_toggled)
        :
        : "cc"
    );
    
    // Восстанавливаем
    __asm__ volatile (
        ".code32\n"
        "pushl %0\n"
        "popfl\n"
        :
        : "r" (flags)
        : "cc"
    );
    
    return ((flags ^ flags_toggled) & (1 << 18)) != 0;
}

// Проверка 286 через попытку очистки битов 12-15
static uint8_t cpu_is_286_or_below(void) {
    uint16_t flags, flags2;
    
    __asm__ volatile (
        "pushf\n"
        "popw %0\n"
        : "=r" (flags)
    );
    
    // На 8086/8088 биты 12-15 всегда 1
    if ((flags & 0xF000) != 0xF000) {
        return 0; // Не 8086/8088
    }
    
    // Пытаемся очистить биты 12-15
    // Используем временную переменную для работы со стеком
    __asm__ volatile (
        "pushf\n"
        "popw %0\n"
        "andw $0x0FFF, %0\n"
        "pushw %0\n"
        "popf\n"
        "pushf\n"
        "popw %1\n"
        : "=r" (flags), "=r" (flags2)
        :
        : "cc"
    );
    
    // Если биты остались 1, это 8086/8088
    // Если очистились, это 286+
    return (flags2 & 0xF000) == 0xF000;
}

// Определение типа процессора
static cpu_type_t cpu_detect_type(void) {
    // 1. Проверяем CPUID (486+)
    if (cpu_has_cpuid()) {
        uint32_t eax, ebx, ecx, edx;
        
        // Получаем информацию через CPUID
        __asm__ volatile (
            ".code32\n"
            "cpuid\n"
            : "=a" (eax), "=b" (ebx), "=c" (ecx), "=d" (edx)
            : "a" (1)
            : "cc"
        );
        
        uint8_t family = (eax >> 8) & 0x0F;
        
        if (family == 4) return CPU_80486;
        if (family >= 5) return CPU_PENTIUM;
        return CPU_80486; // По умолчанию
    }
    
    // 2. Проверяем AC флаг (386+)
    if (cpu_has_ac_flag()) {
        return CPU_80386;
    }
    
    // 3. Проверяем 286 или ниже
    if (cpu_is_286_or_below()) {
        // На 286 нельзя отличить от 8086/8088 простыми методами
        // Возвращаем 286 как более вероятный для BIOS
        return CPU_80286;
    }
    
    // 4. Если ничего не определили
    return CPU_UNKNOWN;
}

// Получение имени процессора
static void cpu_get_name(cpu_info_t* info) {
    switch(info->type) {
        case CPU_8086:
            my_strcpy(info->name, "Intel 8086");
            break;
        case CPU_8088:
            my_strcpy(info->name, "Intel 8088");
            break;
        case CPU_80286:
            my_strcpy(info->name, "Intel 80286");
            break;
        case CPU_80386:
            my_strcpy(info->name, "Intel 80386");
            break;
        case CPU_80486:
            my_strcpy(info->name, "Intel 80486");
            break;
        case CPU_PENTIUM:
            my_strcpy(info->name, "Intel Pentium");
            break;
        default:
            my_strcpy(info->name, "Unknown CPU");
            break;
    }
}

// Основная функция обнаружения
void cpu_detect(cpu_info_t* info) {
    // Очищаем структуру
    info->type = CPU_UNKNOWN;
    info->name[0] = '\0';
    info->has_cpuid = 0;
    info->speed_mhz = 0;
    
    // Определяем тип
    info->type = cpu_detect_type();
    
    // Проверяем CPUID
    info->has_cpuid = cpu_has_cpuid();
    
    // Получаем имя
    cpu_get_name(info);
    
    // Грубая оценка частоты (можно убрать если не нужно)
    switch(info->type) {
        case CPU_8086:
        case CPU_8088:
            info->speed_mhz = 8;
            break;
        case CPU_80286:
            info->speed_mhz = 12;
            break;
        case CPU_80386:
            info->speed_mhz = 33;
            break;
        case CPU_80486:
            info->speed_mhz = 66;
            break;
        case CPU_PENTIUM:
            info->speed_mhz = 133;
            break;
        default:
            info->speed_mhz = 0;
    }
}

// Вывод информации
void cpu_print_info(const cpu_info_t* info, uint8_t x, uint8_t y, uint8_t color) {
    char buffer[64];
    
    // Формируем строку "CPU: [name]"
    my_strcpy(buffer, "CPU: ");
    my_strcpy(buffer + 5, info->name);
    
    // Добавляем частоту если есть
    if (info->speed_mhz > 0) {
        int len = my_strlen(buffer);
        char speed_str[16];
        char* ptr = speed_str;
        uint16_t speed = info->speed_mhz;
        
        // Конвертируем число в строку
        if (speed >= 1000) {
            *ptr++ = (speed / 1000) + '0';
            speed %= 1000;
            *ptr++ = (speed / 100) + '0';
            speed %= 100;
            *ptr++ = (speed / 10) + '0';
            *ptr++ = (speed % 10) + '0';
        } else if (speed >= 100) {
            *ptr++ = (speed / 100) + '0';
            speed %= 100;
            *ptr++ = (speed / 10) + '0';
            *ptr++ = (speed % 10) + '0';
        } else if (speed >= 10) {
            *ptr++ = (speed / 10) + '0';
            *ptr++ = (speed % 10) + '0';
        } else {
            *ptr++ = speed + '0';
        }
        *ptr++ = 'M';
        *ptr++ = 'H';
        *ptr++ = 'z';
        *ptr = '\0';
        
        // Добавляем к основной строке
        buffer[len++] = ' ';
        buffer[len++] = '(';
        my_strcpy(buffer + len, speed_str);
        len += my_strlen(speed_str);
        buffer[len++] = ')';
        buffer[len] = '\0';
    }
    
    // Выводим
    print_string(buffer, x, y, color);
}
