#include "watch.h"
#include "console.h"
#include "cpu.h"

// Внешние функции
extern uint8_t read_cmos(uint8_t reg);
extern void print_string(const char* str, uint8_t x, uint8_t y, uint8_t color);
extern void print_char(char c, uint8_t x, uint8_t y, uint8_t color);
extern void delay(uint32_t count);

// Статические переменные для обновления дисплея
static uint32_t time_update_counter = 0;
static cmos_time_t current_time;

// Конвертация BCD в двоичный формат
uint8_t cmos_bcd_to_bin(uint8_t bcd) {
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

// Ожидание обновления CMOS (для безопасного чтения)
void cmos_wait_for_update(void) {
    // Ждем, пока не закончится обновление
    while (read_cmos(CMOS_STATUS_A) & 0x80) {
        delay(10);
    }
}

// Чтение времени из CMOS
void cmos_read_time(cmos_time_t *time) {
    uint8_t reg_b, hour_format;
    
    // Читаем регистр B для определения формата
    reg_b = read_cmos(CMOS_STATUS_B);
    hour_format = reg_b & 0x02; // Бит 1: 0 = 12-часовой формат, 1 = 24-часовой
    
    do {
        cmos_wait_for_update();
        
        time->second = read_cmos(CMOS_SECOND);
        time->minute = read_cmos(CMOS_MINUTE);
        time->hour = read_cmos(CMOS_HOUR);
        time->day = read_cmos(CMOS_DAY);
        time->month = read_cmos(CMOS_MONTH);
        time->year = read_cmos(CMOS_YEAR);
        time->weekday = read_cmos(CMOS_WEEKDAY);
        
        // Пытаемся прочитать век (может не поддерживаться)
        uint8_t century = read_cmos(CMOS_CENTURY);
        
        // Конвертируем из BCD
        time->second = cmos_bcd_to_bin(time->second);
        time->minute = cmos_bcd_to_bin(time->minute);
        
        // Обработка часов в зависимости от формата
        if (!hour_format && (time->hour & 0x80)) {
            // 12-часовой формат с PM флагом
            time->hour = cmos_bcd_to_bin(time->hour & 0x7F);
            // Можно добавить обработку AM/PM при необходимости
        } else {
            time->hour = cmos_bcd_to_bin(time->hour);
        }
        
        time->day = cmos_bcd_to_bin(time->day);
        time->month = cmos_bcd_to_bin(time->month);
        time->year = cmos_bcd_to_bin(time->year);
        
        // Собираем полный год
        if (century != 0) {
            uint8_t century_bin = cmos_bcd_to_bin(century);
            time->year = century_bin * 100 + time->year;
        } else {
            // Если век не указан, предполагаем 20xx для лет < 80
            // и 19xx для лет >= 80 (Y2K совместимость)
            if (time->year < 80) {
                time->year = 2000 + time->year;
            } else {
                time->year = 1900 + time->year;
            }
        }
        
    } while (time->second != cmos_bcd_to_bin(read_cmos(CMOS_SECOND)));
}

// Отображение времени
void cmos_display_time(void) {
    char time_str[9];
    
    // Форматируем время HH:MM:SS
    time_str[0] = '0' + current_time.hour / 10;
    time_str[1] = '0' + current_time.hour % 10;
    time_str[2] = ':';
    time_str[3] = '0' + current_time.minute / 10;
    time_str[4] = '0' + current_time.minute % 10;
    time_str[5] = ':';
    time_str[6] = '0' + current_time.second / 10;
    time_str[7] = '0' + current_time.second % 10;
    time_str[8] = '\0';
    
    // Отображаем время в правом нижнем углу
    print_string("Time: ", 59, 22, 0x0F);
    print_string(time_str, 65, 22, 0x0F);
}

// Отображение даты
void cmos_display_date(void) {
    char date_str[12];
    const char* month_names[] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };
    
    const char* day_names[] = {
        "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
    };
    
    // Формат: Wed 15-Jan-2025
    // День недели (первые 3 буквы)
    date_str[0] = day_names[current_time.weekday % 7][0];
    date_str[1] = day_names[current_time.weekday % 7][1];
    date_str[2] = day_names[current_time.weekday % 7][2];
    date_str[3] = ' ';
    
    // Число
    date_str[4] = '0' + current_time.day / 10;
    date_str[5] = '0' + current_time.day % 10;
    date_str[6] = '-';
    
    // Месяц (3 буквы)
    date_str[7] = month_names[current_time.month - 1][0];
    date_str[8] = month_names[current_time.month - 1][1];
    date_str[9] = month_names[current_time.month - 1][2];
    date_str[10] = '-';
    
    // Год
    date_str[11] = '0' + (current_time.year / 1000) % 10;
    date_str[12] = '0' + (current_time.year / 100) % 10;
    date_str[13] = '0' + (current_time.year / 10) % 10;
    date_str[14] = '0' + current_time.year % 10;
    date_str[15] = '\0';
    
    // Отображаем дату
    print_string("Date: ", 59, 23, 0x0F);
    print_string(date_str, 65, 23, 0x0F);
}

// Обновление дисплея времени
void cmos_update_display(void) {
    time_update_counter++;
    
    // Обновляем каждые ~500 мс (настраиваемо)
    if (time_update_counter % 5000 == 0) {
        cmos_read_time(&current_time);
        cmos_display_time();
        cmos_display_date();
    }
}

// Инициализация часов
void watch_init(void) {
    // Читаем начальное время
    cmos_read_time(&current_time);
    
    // Инициализируем дисплей
    cmos_display_time();
    cmos_display_date();
    
    // Выводим информацию о поддержке часов
    print_string("RTC: Active", 22, 14, 0x0A);
}
