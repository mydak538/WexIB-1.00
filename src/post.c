#include "../include/post.h"

// Глобальные переменные для результатов тестов
static uint8_t post_results = 0;
static uint16_t* video_mem = (uint16_t*)0xB8000;

// Вспомогательные функции
static uint8_t inb(uint16_t port) {
    uint8_t result;
    asm volatile ("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

static void outb(uint16_t port, uint8_t value) {
    asm volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

// Объявляем внешние функции из console.h
extern void delay(uint32_t milliseconds);

// Определения констант
#define KEYBOARD_STATUS_PORT 0x64
#define KEYBOARD_DATA_PORT 0x60
#define IDE_STATUS 0x1F7
#define CMOS_ADDRESS 0x70
#define CMOS_DATA 0x71
#define PIT_COMMAND 0x43
#define PIT_CHANNEL2 0x42
#define SPEAKER_PORT 0x61

uint8_t run_post(void) {
    post_results = POST_SUCCESS;
    
    // 1. Тест процессора
    post_cpu_test();
    if (post_results != POST_SUCCESS) return post_results;
    
    // 2. Тест памяти
    post_memory_test();
    if (post_results != POST_SUCCESS) return post_results;
    
    // 3. Тест видео
    post_video_test();
    if (post_results != POST_SUCCESS) return post_results;
    
    // 4. Тест клавиатуры
    post_keyboard_test();
    if (post_results != POST_SUCCESS) return post_results;
    
    // 5. Тест диска
    post_disk_test();
    if (post_results != POST_SUCCESS) return post_results;
    
    // 6. Тест CMOS
    post_cmos_test();
    
    // Успешный звуковой сигнал
    beep(1000, 100);
    delay(500); // Используем delay из console.h
    beep(1500, 100);
    
    return post_results;
}

void post_cpu_test(void) {
    // Проверка базовой функциональности CPU
    uint32_t test_value = 0;
    
    // Тест арифметических операций
    test_value = 1234 + 5678;
    if (test_value != 6912) {
        post_results = POST_CPU_FAIL;
        return;
    }
    
    // Тест битовых операций
    test_value = 0x55AA & 0x0FF0;
    if (test_value != 0x05A0) {
        post_results = POST_CPU_FAIL;
        return;
    }
    
    // Тест сдвигов
    test_value = 0x1234 << 4;
    if (test_value != 0x12340) {
        post_results = POST_CPU_FAIL;
        return;
    }
    
    // Успешный звуковой сигнал для CPU
    beep(800, 50);
}

void post_memory_test(void) {
    // Базовый тест памяти - проверка записи/чтения
    volatile uint32_t* test_addr = (volatile uint32_t*)0x1000;
    uint32_t test_pattern = 0x55AA1234;
    
    // Тест записи и чтения
    *test_addr = test_pattern;
    if (*test_addr != test_pattern) {
        post_results = POST_MEMORY_FAIL;
        return;
    }
    
    // Тест инверсного паттерна
    test_pattern = ~test_pattern;
    *test_addr = test_pattern;
    if (*test_addr != test_pattern) {
        post_results = POST_MEMORY_FAIL;
        return;
    }
    
    // Успешный звуковой сигнал для памяти
    beep(900, 50);
}

void post_video_test(void) {
    // Тест видеопамяти
    uint16_t test_pattern = 0x1F41; // Синий фон, белый 'A'
    
    // Записываем тестовый символ в несколько мест
    for (int i = 0; i < 80; i += 10) {
        video_mem[i] = test_pattern;
    }
    
    // Проверяем, что записалось
    for (int i = 0; i < 80; i += 10) {
        if (video_mem[i] != test_pattern) {
            post_results = POST_VIDEO_FAIL;
            return;
        }
        // Очищаем
        video_mem[i] = 0x0720;
    }
    
    // Успешный звуковой сигнал для видео
    beep(1000, 50);
}

void post_keyboard_test(void) {
    // Тест контроллера клавиатуры - более простая проверка
    uint8_t status = inb(KEYBOARD_STATUS_PORT);
    
    // Проверяем, что контроллер вообще отвечает (не 0xFF или 0x00)
    if (status == 0xFF || status == 0x00) {
        post_results = POST_KEYBOARD_FAIL;
        return;
    }
    
    // Пробуем очистить буфер клавиатуры
    int timeout = 1000;
    while ((inb(KEYBOARD_STATUS_PORT) & 0x01) && timeout > 0) {
        // Читаем и отбрасываем данные из буфера
        inb(KEYBOARD_DATA_PORT);
        delay(1); // Используем delay из console.h
        timeout--;
    }
    
    // Проверяем, можем ли мы прочитать статус после очистки
    status = inb(KEYBOARD_STATUS_PORT);
    
    // Если статус все еще показывает ошибку, считаем тест неудачным
    if ((status & 0x20) != 0) { // Проверка ошибки передачи
        post_results = POST_KEYBOARD_FAIL;
        return;
    }
    
    // Успешный звуковой сигнал для клавиатуры
    beep(1100, 50);
}

void post_disk_test(void) {
    // Тест наличия IDE контроллера
    uint8_t status = inb(IDE_STATUS);
    
    // Проверяем, что контроллер отвечает
    if (status == 0xFF || status == 0x00) {
        // Контроллер не отвечает
        post_results = POST_DISK_FAIL;
        return;
    }
    
    // Проверяем бит busy
    if ((status & 0x80) != 0) {
        // Диск занят слишком долго
        post_results = POST_DISK_FAIL;
        return;
    }
    
    // Успешный звуковой сигнал для диска
    beep(1200, 50);
}

void post_cmos_test(void) {
    // Тест CMOS
    uint8_t original_reg = inb(CMOS_ADDRESS);
    uint8_t test_value = 0x55;
    
    // Пытаемся записать и прочитать из CMOS
    outb(CMOS_ADDRESS, 0x0E); // Регистр диагностики
    outb(CMOS_DATA, test_value);
    
    outb(CMOS_ADDRESS, 0x0E);
    uint8_t read_value = inb(CMOS_DATA);
    
    // Восстанавливаем оригинальное значение
    outb(CMOS_ADDRESS, 0x0E);
    outb(CMOS_DATA, original_reg);
    
    if (read_value != test_value) {
        post_results = POST_CMOS_FAIL;
        return;
    }
    
    // Успешный звуковой сигнал для CMOS
    beep(1300, 50);
}

void show_post_error(uint8_t error_code) {
    const char* error_messages[] = {
        "POST: All tests passed",
        "CPU Test Failed",
        "Memory Test Failed", 
        "Video Test Failed",
        "Keyboard Test Failed",
        "Disk Test Failed",
        "CMOS Test Failed"
    };
    
    // Выводим сообщение об ошибке
    const char* msg = error_messages[error_code];
    for (int i = 0; msg[i] != '\0'; i++) {
        video_mem[i] = 0x4F00 | msg[i]; // Красный фон
    }
    
    // Издаем звук ошибки
    for (int i = 0; i < 3; i++) {
        beep(300, 200);
        delay(100); // Используем delay из console.h
    }
}

void beep(uint32_t frequency, uint32_t duration) {
    // Устанавливаем частоту
    uint32_t divisor = 1193180 / frequency;
    outb(PIT_COMMAND, 0xB6);
    outb(PIT_CHANNEL2, divisor & 0xFF);
    outb(PIT_CHANNEL2, (divisor >> 8) & 0xFF);
    
    // Включаем динамик
    uint8_t tmp = inb(SPEAKER_PORT);
    outb(SPEAKER_PORT, tmp | 0x03);
    
    // Ждем
    delay(duration);
    
    // Выключаем динамик
    tmp = inb(SPEAKER_PORT);
    outb(SPEAKER_PORT, tmp & 0xFC);
}

void post_delay(uint32_t count) {
    for (volatile uint32_t i = 0; i < count; i++);
}
