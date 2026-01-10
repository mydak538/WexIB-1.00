#ifndef CPU_H
#define CPU_H

#include <stdint.h>

// Типы процессоров
typedef enum {
    CPU_UNKNOWN = 0,
    // Intel
    CPU_8086,
    CPU_8088,
    CPU_80286,
    CPU_80386,
    CPU_80486,
    CPU_PENTIUM,
    CPU_PENTIUM_MMX,
    CPU_PENTIUM_PRO,
    CPU_PENTIUM_II,
    CPU_PENTIUM_III,
    CPU_PENTIUM_4,
    CPU_INTEL_CORE,
    CPU_INTEL_CORE2,
    CPU_INTEL_CORE_I3,
    CPU_INTEL_CORE_I5,
    CPU_INTEL_CORE_I7,
    CPU_INTEL_CORE_I9,
    // AMD
    CPU_AMD_AM486,
    CPU_AMD_K5,
    CPU_AMD_K6,
    CPU_AMD_K6_2,
    CPU_AMD_K6_3,
    CPU_AMD_K7_ATHLON,
    CPU_AMD_K7_DURON,
    CPU_AMD_K7_THUNDERBIRD,
    CPU_AMD_RYZEN,
    CPU_AMD_RYZEN_THREADRIPPER,
    // Cyrix
    CPU_CYRIX_6x86,
    CPU_CYRIX_MII,
    // VIA
    CPU_VIA_C3,
    CPU_VIA_C7,
    // Виртуальные
    CPU_QEMU,
    CPU_VIRTUALBOX,
    CPU_VMWARE,
    CPU_HYPERV,
    // Особые
    CPU_TRANSMETA_CRUSOE,
    CPU_IBM_POWERPC,
    CPU_ARM_EMULATED
} cpu_type_t;

// Структура информации о процессоре
typedef struct {
    cpu_type_t type;
    char name[48];
    char vendor[16];
    char model[32];
    uint8_t has_cpuid;
    uint16_t family;
    uint16_t model_num;
    uint16_t stepping;
    uint32_t features;
    uint16_t speed_mhz;
    uint8_t is_virtual;
    uint8_t cores;
} cpu_info_t;

// Прототипы функций
void cpu_detect(cpu_info_t* info);
void cpu_print_info(const cpu_info_t* info, uint8_t x, uint8_t y, uint8_t color);

#endif // CPU_H
