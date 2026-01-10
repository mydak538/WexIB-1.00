#ifndef POST_H
#define POST_H

#include <stdint.h>

// Результаты POST тестов
#define POST_SUCCESS 0
#define POST_CPU_FAIL 1
#define POST_MEMORY_FAIL 2
#define POST_VIDEO_FAIL 3
#define POST_KEYBOARD_FAIL 4
#define POST_DISK_FAIL 5
#define POST_CMOS_FAIL 6

// Функции POST
uint8_t run_post(void);
void post_cpu_test(void);
void post_memory_test(void);
void post_video_test(void);
void post_keyboard_test(void);
void post_disk_test(void);
void post_cmos_test(void);
void show_post_error(uint8_t error_code);
void beep(uint32_t frequency, uint32_t duration);
void post_delay(uint32_t count); // Изменено имя для избежания конфликтов

#endif // POST_H
