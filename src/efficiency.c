#include "efficiency.h"
#include "console.h"
#include "ports.h" 
#include <stdint.h>

// Определяем глобальные переменные
power_mode_t current_power_mode = POWER_MODE_BALANCED;

// Названия режимов
const char* power_mode_names[] = {
    "Max Performance",
    "Balanced",
    "Power Saving",
    "Min Power"
};

// Описания режимов
const char* power_mode_descriptions[] = {
    "Maximum performance, highest power consumption",
    "Balanced performance and power usage",
    "Reduced performance for power saving", 
    "Minimum power, basic functionality only"
};

// Вспомогательные функции
static void int_to_str(uint32_t value, char* buffer) {
    char* ptr = buffer;
    char* ptr1 = buffer;
    char tmp_char;
    
    // Конвертируем число в строку
    do {
        *ptr++ = '0' + (value % 10);
        value /= 10;
    } while (value);
    
    *ptr-- = '\0';
    
    // Разворачиваем строку
    while(ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }
}

void efficiency_init(void) {
    // Загружаем настройки из CMOS
    outb(0x70, 0x30);
    uint8_t saved_mode = inb(0x71);
    
    if (saved_mode <= POWER_MODE_MIN_POWER) {
        current_power_mode = (power_mode_t)saved_mode;
    }
}

void set_power_mode(power_mode_t mode) {
    if (mode > POWER_MODE_MIN_POWER) {
        mode = POWER_MODE_BALANCED;
    }
    
    current_power_mode = mode;
    apply_power_settings();
    save_power_settings_to_cmos();
}

void apply_power_settings(void) {
    // Базовая реализация
    switch(current_power_mode) {
        case POWER_MODE_MAX_PERFORMANCE:
            // Максимальная производительность
            break;
        case POWER_MODE_BALANCED:
            // Сбалансированный режим
            break;
        case POWER_MODE_POWER_SAVING:
        case POWER_MODE_MIN_POWER:
            // Экономия энергии
            break;
    }
}

void power_management_menu(void) {
    uint8_t selected = 0;
    
    clear_screen(0x07);
    print_string("POWER MANAGEMENT SETTINGS", 28, 1, 0x0F);
    print_string("==========================", 28, 2, 0x0F);
    
    while(1) {
        // Отрисовка меню
        for(int i = 0; i < 4; i++) {
            uint8_t color = (i == selected) ? 0x1F : 0x07;
            if (i == selected) {
                print_string(">", 25, 5 + i, color);
            } else {
                print_string(" ", 25, 5 + i, color);
            }
            
            print_string(power_mode_names[i], 27, 5 + i, color);
            
            // Галочка для текущего режима
            if (i == current_power_mode) {
                print_string(" [X]", 50, 5 + i, 0x0A);
            } else {
                print_string(" [ ]", 50, 5 + i, 0x07);
            }
        }
        
        // Описание выбранного режима
        print_string("Description: ", 25, 12, 0x0F);
        print_string(power_mode_descriptions[selected], 25, 13, 0x07);
        
        // Текущий статус
        print_string("Current mode: ", 25, 15, 0x0F);
        print_string(power_mode_names[current_power_mode], 40, 15, 
                    current_power_mode == POWER_MODE_MAX_PERFORMANCE ? 0x0E :
                    current_power_mode == POWER_MODE_BALANCED ? 0x0A :
                    current_power_mode == POWER_MODE_POWER_SAVING ? 0x0B : 0x0C);
        
        print_string("ENTER: Select  ESC: Return", 20, 20, 0x07);
        
        uint8_t scancode = keyboard_read();
        if (scancode == 0) {
            delay(10000);
            continue;
        }
        
        if (scancode & 0x80) continue;
        
        if (scancode == KEY_UP) {
            if (selected > 0) selected--;
        }
        else if (scancode == KEY_DOWN) {
            if (selected < 3) selected++;
        }
        else if (scancode == KEY_ENTER) {
            set_power_mode(selected);
            
            // Обновляем галочки
            for(int i = 0; i < 4; i++) {
                print_string("    ", 50, 5 + i, 0x07);
                if (i == current_power_mode) {
                    print_string("[X]", 50, 5 + i, 0x0A);
                } else {
                    print_string("[ ]", 50, 5 + i, 0x07);
                }
            }
            
            // Обновляем текущий статус
            print_string("                ", 40, 15, 0x07);
            print_string(power_mode_names[current_power_mode], 40, 15, 
                        current_power_mode == POWER_MODE_MAX_PERFORMANCE ? 0x0E :
                        current_power_mode == POWER_MODE_BALANCED ? 0x0A :
                        current_power_mode == POWER_MODE_POWER_SAVING ? 0x0B : 0x0C);
        }
        else if (scancode == KEY_ESC) {
            return;
        }
    }
}

void cycle_power_mode(void) {
    uint8_t next_mode = (current_power_mode + 1) % 4;
    set_power_mode(next_mode);
}

void auto_power_management(void) {
    // Можно добавить автоматическое управление позже
}

void show_power_info(uint8_t x, uint8_t y) {
    print_string("Power Mode: ", x, y, 0x0F);
    print_string(power_mode_names[current_power_mode], x + 13, y, 
                current_power_mode == POWER_MODE_MAX_PERFORMANCE ? 0x0E :
                current_power_mode == POWER_MODE_BALANCED ? 0x0A :
                current_power_mode == POWER_MODE_POWER_SAVING ? 0x0B : 0x0C);
}

void save_power_settings_to_cmos(void) {
    outb(0x70, 0x30);
    outb(0x71, current_power_mode);
}

void load_power_settings_from_cmos(void) {
    outb(0x70, 0x30);
    uint8_t saved_mode = inb(0x71);
    
    if (saved_mode <= POWER_MODE_MIN_POWER) {
        current_power_mode = (power_mode_t)saved_mode;
    }
}
