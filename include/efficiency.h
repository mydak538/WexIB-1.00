#ifndef EFFICIENCY_H
#define EFFICIENCY_H

#include <stdint.h>

// Режимы энергосбережения
typedef enum {
    POWER_MODE_MAX_PERFORMANCE = 0,  // Максимальная производительность
    POWER_MODE_BALANCED = 1,         // Сбалансированный (по умолчанию)
    POWER_MODE_POWER_SAVING = 2,     // Экономия энергии
    POWER_MODE_MIN_POWER = 3,        // Минимум энергии
} power_mode_t;

// Настройки для каждого режима
typedef struct {
    const char* name;
    uint8_t cpu_speed;      // 0-100% от максимальной
    uint8_t screen_brightness; // 0-100%
    uint8_t fan_speed;      // 0-100%
    uint8_t usb_power;      // 0-1 (вкл/выкл)
    uint8_t hdd_timeout;    // сек до отключения HDD
    uint8_t screen_timeout; // сек до отключения экрана
    uint8_t performance_boost; // 0-1
} power_settings_t;

// Статистика энергопотребления
typedef struct {
    uint32_t total_uptime;     // секунд работы
    uint32_t power_save_time;  // секунд в режиме экономии
    uint8_t temperature;       // текущая температура °C
    uint8_t cpu_usage;         // текущая загрузка CPU %
    uint16_t voltage;          // mV
} power_stats_t;

// Глобальные переменные
extern power_mode_t current_power_mode;
extern power_settings_t power_settings[4];
extern power_stats_t power_stats;

// Функции
void efficiency_init(void);
void set_power_mode(power_mode_t mode);
void cycle_power_mode(void);
void apply_power_settings(void);
void update_power_stats(void);
void show_power_info(uint8_t x, uint8_t y);
void power_management_menu(void);
void auto_power_management(void);

// Аппаратные функции
void set_cpu_speed(uint8_t percent);
void set_screen_brightness(uint8_t percent);
void control_fan_speed(uint8_t percent);
void toggle_usb_power(uint8_t enable);
void enable_performance_boost(uint8_t enable);

// CMOS сохранение настроек
void save_power_settings_to_cmos(void);
void load_power_settings_from_cmos(void);

// Утилиты
uint8_t read_temperature(void);
uint16_t read_voltage(void);
uint8_t estimate_cpu_usage(void);

#endif // EFFICIENCY_H
