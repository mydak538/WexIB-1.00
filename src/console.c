#include "console.h"
#include "post.h"

// Debug console state
static uint8_t debug_line = 3;

void clear_debug_screen(void) {
    debug_line = 3;
    clear_screen(0x00); // Черный фон
}

void log_debug_message(const char* message, uint8_t color) {
    if (debug_line >= 24) {
        // Simple scroll - just reset
        clear_debug_screen();
        print_string("=== BIOS DEBUG CONSOLE ===", 25, 0, DEBUG_COLOR_INFO);
        print_string("F1:POST  F2:CMOS  F3:Memory  F4:CPU  ESC:Exit", 15, 1, DEBUG_COLOR_NORMAL);
    }
    
    print_string(message, 0, debug_line, color);
    debug_line++;
}

void log_debug_hex(const char* label, uint32_t value, uint8_t color) {
    char buffer[64];
    char hex_chars[] = "0123456789ABCDEF";
    char hex[9];
    
    for(int i = 7; i >= 0; i--) {
        hex[i] = hex_chars[value & 0xF];
        value >>= 4;
    }
    hex[8] = '\0';
    
    // Format: "Label: 0x12345678"
    int pos = 0;
    while (*label) buffer[pos++] = *label++;
    buffer[pos++] = ':';
    buffer[pos++] = ' ';
    buffer[pos++] = '0';
    buffer[pos++] = 'x';
    for(int i = 0; i < 8; i++) buffer[pos++] = hex[i];
    buffer[pos] = '\0';
    
    log_debug_message(buffer, color);
}

void dump_memory_map(void) {
    log_debug_message("Memory Map:", DEBUG_COLOR_INFO);
    log_debug_hex("Video Memory", 0xB8000, DEBUG_COLOR_DEBUG);
    log_debug_hex("BIOS ROM", 0xF0000, DEBUG_COLOR_DEBUG);
    log_debug_hex("Extended BIOS", 0xE0000, DEBUG_COLOR_DEBUG);
    
    // Simple memory test
    uint32_t* test_addr = (uint32_t*)0x1000;
    uint32_t original = *test_addr;
    *test_addr = 0x55AA1234;
    
    if (*test_addr == 0x55AA1234) {
        *test_addr = original;
        log_debug_message("Base RAM: OK", DEBUG_COLOR_SUCCESS);
    } else {
        log_debug_message("Base RAM: FAIL", DEBUG_COLOR_ERROR);
    }
    
    // Test extended memory
    test_addr = (uint32_t*)0x100000;
    original = *test_addr;
    *test_addr = 0x12345678;
    
    if (*test_addr == 0x12345678) {
        *test_addr = original;
        log_debug_message("Extended RAM: OK", DEBUG_COLOR_SUCCESS);
    } else {
        log_debug_message("Extended RAM: FAIL", DEBUG_COLOR_WARNING);
    }
}

void show_system_registers(void) {
    log_debug_message("System Registers:", DEBUG_COLOR_INFO);
    
    uint32_t cr0, cr2, cr3;
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
    __asm__ volatile("mov %%cr2, %0" : "=r"(cr2));
    __asm__ volatile("mov %%cr3, %0" : "=r"(cr3));
    
    log_debug_hex("CR0", cr0, DEBUG_COLOR_DEBUG);
    log_debug_hex("CR2", cr2, DEBUG_COLOR_DEBUG);
    log_debug_hex("CR3", cr3, DEBUG_COLOR_DEBUG);
    
    uint16_t cs, ds, es, ss;
    __asm__ volatile("mov %%cs, %0" : "=r"(cs));
    __asm__ volatile("mov %%ds, %0" : "=r"(ds));
    __asm__ volatile("mov %%es, %0" : "=r"(es));
    __asm__ volatile("mov %%ss, %0" : "=r"(ss));
    
    log_debug_hex("CS", cs, DEBUG_COLOR_DEBUG);
    log_debug_hex("DS", ds, DEBUG_COLOR_DEBUG);
    log_debug_hex("ES", es, DEBUG_COLOR_DEBUG);
    log_debug_hex("SS", ss, DEBUG_COLOR_DEBUG);
}

void dump_cmos_registers(void) {
    log_debug_message("CMOS Dump (first 32 bytes):", DEBUG_COLOR_INFO);
    
    for (uint8_t reg = 0; reg < 32; reg++) {
        char line[32];
        uint8_t value;
        
        __asm__ volatile(
            "outb %1, $0x70\n"
            "inb $0x71, %0"
            : "=a"(value)
            : "a"(reg)
        );
        
        line[0] = '0'; line[1] = 'x';
        line[2] = (reg >> 4) < 10 ? (reg >> 4) + '0' : (reg >> 4) - 10 + 'A';
        line[3] = (reg & 0xF) < 10 ? (reg & 0xF) + '0' : (reg & 0xF) - 10 + 'A';
        line[4] = ':'; line[5] = ' ';
        line[6] = '0'; line[7] = 'x';
        line[8] = (value >> 4) < 10 ? (value >> 4) + '0' : (value >> 4) - 10 + 'A';
        line[9] = (value & 0xF) < 10 ? (value & 0xF) + '0' : (value & 0xF) - 10 + 'A';
        line[10] = '\0';
        
        log_debug_message(line, DEBUG_COLOR_DEBUG);
    }
}

void debug_console(void) {
    clear_debug_screen();
    print_string("=== BIOS DEBUG CONSOLE ===", 25, 0, DEBUG_COLOR_INFO);
    print_string("F1:POST  F2:CMOS  F3:Memory  F4:CPU  ESC:Exit", 15, 1, DEBUG_COLOR_NORMAL);
    
    log_debug_message("Debug console ready", DEBUG_COLOR_SUCCESS);
    
    while(1) {
        uint8_t scancode = keyboard_read();
        if (scancode == 0) {
            delay(10000);
            continue;
        }
        
        if (scancode & 0x80) continue;
        
        switch(scancode) {
            case 0x3B: // F1 - Run POST
                log_debug_message(">>> Running POST...", DEBUG_COLOR_WARNING);
                uint8_t post_result = run_post();
                if (post_result == 0) {
                    log_debug_message("POST: SUCCESS", DEBUG_COLOR_SUCCESS);
                } else {
                    log_debug_message("POST: FAILED", DEBUG_COLOR_ERROR);
                    log_debug_hex("Error code", post_result, DEBUG_COLOR_ERROR);
                }
                break;
                
            case 0x3C: // F2 - CMOS dump
                log_debug_message(">>> Dumping CMOS...", DEBUG_COLOR_WARNING);
                dump_cmos_registers();
                break;
                
            case 0x3D: // F3 - Memory info
                log_debug_message(">>> Memory map...", DEBUG_COLOR_WARNING);
                dump_memory_map();
                break;
                
            case 0x3E: // F4 - CPU info
                log_debug_message(">>> CPU registers...", DEBUG_COLOR_WARNING);
                show_system_registers();
                break;
                
            case KEY_ESC:
                log_debug_message("Exiting debug console...", DEBUG_COLOR_INFO);
                delay(300000);
                return;
        }
    }
}

void dev_tools_menu(void) {
    uint8_t selected = 0;
    const char* dev_items[] = {
        "Debug Console",
        "System Registers", 
        "Memory Map",
        "CMOS Dump"
    };
    const int items_count = 4;
    
    clear_screen(0x00); // Черный фон
    print_string("DEVELOPER TOOLS", 35, 1, 0x0F);
    print_string("===============", 35, 2, 0x0F);
    print_string("For debugging and system analysis", 25, 4, 0x07);
    
    while(1) {
        for(int i = 0; i < items_count; i++) {
            uint8_t color = (i == selected) ? 0x1F : 0x07;
            print_string(i == selected ? ">" : " ", 30, 6 + i, color);
            print_string(dev_items[i], 32, 6 + i, color);
        }
        
        print_string("ENTER: Select  ESC: Return", 25, 20, 0x07);
        
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
            if (selected < items_count - 1) selected++;
        }
        else if (scancode == KEY_ENTER) {
            switch(selected) {
                case 0: 
                    debug_console(); 
                    break;
                case 1: 
                    clear_debug_screen();
                    log_debug_message("System Registers:", DEBUG_COLOR_INFO);
                    show_system_registers();
                    log_debug_message("Press any key...", DEBUG_COLOR_NORMAL);
                    keyboard_read();
                    break;
                case 2: 
                    clear_debug_screen();
                    log_debug_message("Memory Map:", DEBUG_COLOR_INFO);
                    dump_memory_map();
                    log_debug_message("Press any key...", DEBUG_COLOR_NORMAL);
                    keyboard_read();
                    break;
                case 3: 
                    clear_debug_screen();
                    log_debug_message("CMOS Dump:", DEBUG_COLOR_INFO);
                    dump_cmos_registers();
                    log_debug_message("Press any key...", DEBUG_COLOR_NORMAL);
                    keyboard_read();
                    break;
            }
            // Redraw menu
            clear_screen(0x00);
            print_string("DEVELOPER TOOLS", 35, 1, 0x0F);
            print_string("===============", 35, 2, 0x0F);
            print_string("For debugging and system analysis", 25, 4, 0x07);
        }
        else if (scancode == KEY_ESC) {
            return;
        }
    }
}
