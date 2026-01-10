#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdint.h>

#define DEBUG_COLOR_NORMAL  0x07
#define DEBUG_COLOR_SUCCESS 0x0A
#define DEBUG_COLOR_ERROR   0x0C
#define DEBUG_COLOR_WARNING 0x0E
#define DEBUG_COLOR_INFO    0x0B
#define DEBUG_COLOR_DEBUG   0x09

#define KEY_UP 0x48
#define KEY_DOWN 0x50
#define KEY_ENTER 0x1C
#define KEY_ESC 0x01

void dev_tools_menu(void);
void debug_console(void);
void show_system_registers(void);
void dump_memory_map(void);
void dump_cmos_registers(void);
void log_debug_message(const char* message, uint8_t color);
void log_debug_hex(const char* label, uint32_t value, uint8_t color);
void clear_debug_screen(void);

extern void print_string(const char* str, uint8_t x, uint8_t y, uint8_t color);
extern void clear_screen(uint8_t color);
extern uint8_t keyboard_read(void);
extern void delay(uint32_t count);

#endif
