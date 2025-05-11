#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include "../../include/fuzzkrieg.h"

// Crash types
typedef enum {
    CRASH_TYPE_NULL_PTR,
    CRASH_TYPE_USE_AFTER_FREE,
    CRASH_TYPE_DOUBLE_FREE,
    CRASH_TYPE_BUFFER_OVERFLOW,
    CRASH_TYPE_INTEGER_OVERFLOW,
    CRASH_TYPE_TYPE_CONFUSION,
    CRASH_TYPE_RACE_CONDITION,
    CRASH_TYPE_KERNEL_PANIC,    // iOS 18 specific
    CRASH_TYPE_MEMORY_CORRUPTION, // iOS 18 specific
    CRASH_TYPE_UNKNOWN
} crash_type_t;

// Crash information structure
typedef struct {
    crash_type_t type;
    char *description;
    char *stack_trace;
    char *registers;
    char *memory_map;
    uint64_t timestamp;
    char *testcase_path;
    char *crash_log_path;
    char *ios_version;  // Added for iOS version tracking
} crash_info_t;

// Initialize crash analyzer
int crash_analyzer_init(void) {
    // Create crash analysis directory
    mkdir("crashes", 0755);
    mkdir("crashes/logs", 0755);
    mkdir("crashes/testcases", 0755);
    mkdir("crashes/ios18", 0755);  // iOS 18 specific directory
    return 0;
}

// Analyze crash log
crash_type_t analyze_crash_log(const char *log_path) {
    if (!log_path) {
        return CRASH_TYPE_UNKNOWN;
    }

    FILE *f = fopen(log_path, "r");
    if (!f) {
        return CRASH_TYPE_UNKNOWN;
    }

    char line[1024];
    crash_type_t type = CRASH_TYPE_UNKNOWN;

    while (fgets(line, sizeof(line), f)) {
        // Check for iOS 18 specific crash patterns
        if (strstr(line, "EXC_BAD_ACCESS")) {
            if (strstr(line, "KERN_INVALID_ADDRESS")) {
                type = CRASH_TYPE_NULL_PTR;
            } else if (strstr(line, "KERN_PROTECTION_FAILURE")) {
                type = CRASH_TYPE_USE_AFTER_FREE;
            } else if (strstr(line, "KERN_MEMORY_CORRUPTION")) {
                type = CRASH_TYPE_MEMORY_CORRUPTION;
            }
        } else if (strstr(line, "EXC_BAD_INSTRUCTION")) {
            if (strstr(line, "KERN_INVALID_ARGUMENT")) {
                type = CRASH_TYPE_TYPE_CONFUSION;
            }
        } else if (strstr(line, "EXC_ARITHMETIC")) {
            type = CRASH_TYPE_INTEGER_OVERFLOW;
        } else if (strstr(line, "EXC_GUARD")) {
            type = CRASH_TYPE_BUFFER_OVERFLOW;
        } else if (strstr(line, "EXC_RESOURCE")) {
            type = CRASH_TYPE_RACE_CONDITION;
        } else if (strstr(line, "KERNEL_PANIC")) {
            type = CRASH_TYPE_KERNEL_PANIC;
        }
    }

    fclose(f);
    return type;
}

// Extract stack trace from crash log
char *extract_stack_trace(const char *log_path) {
    if (!log_path) {
        return NULL;
    }

    FILE *f = fopen(log_path, "r");
    if (!f) {
        return NULL;
    }

    char *stack_trace = malloc(8192);  // Increased buffer size for iOS 18
    if (!stack_trace) {
        fclose(f);
        return NULL;
    }

    stack_trace[0] = '\0';
    char line[1024];
    int in_stack_trace = 0;

    while (fgets(line, sizeof(line), f)) {
        if (strstr(line, "Thread 0 Crashed:") || strstr(line, "Kernel Panic Stack:")) {
            in_stack_trace = 1;
            continue;
        }

        if (in_stack_trace) {
            if (line[0] == '\n' || line[0] == '\0') {
                break;
            }
            strcat(stack_trace, line);
        }
    }

    fclose(f);
    return stack_trace;
}

// Extract register values from crash log
char *extract_registers(const char *log_path) {
    if (!log_path) {
        return NULL;
    }

    FILE *f = fopen(log_path, "r");
    if (!f) {
        return NULL;
    }

    char *registers = malloc(2048);  // Increased buffer size for iOS 18
    if (!registers) {
        fclose(f);
        return NULL;
    }

    registers[0] = '\0';
    char line[1024];
    int in_registers = 0;

    while (fgets(line, sizeof(line), f)) {
        if (strstr(line, "Thread 0 crashed with ARM Thread State") || 
            strstr(line, "Kernel State:")) {
            in_registers = 1;
            continue;
        }

        if (in_registers) {
            if (line[0] == '\n' || line[0] == '\0') {
                break;
            }
            strcat(registers, line);
        }
    }

    fclose(f);
    return registers;
}

// Generate crash report
int generate_crash_report(crash_info_t *info) {
    if (!info) {
        return -1;
    }

    char report_path[256];
    snprintf(report_path, sizeof(report_path), "crashes/ios18/crash_%llu.txt", info->timestamp);

    FILE *f = fopen(report_path, "w");
    if (!f) {
        return -1;
    }

    // Write crash report
    fprintf(f, "iOS 18 Crash Report\n");
    fprintf(f, "==================\n\n");
    fprintf(f, "Timestamp: %llu\n", info->timestamp);
    fprintf(f, "Type: %d\n", info->type);
    fprintf(f, "Description: %s\n", info->description);
    fprintf(f, "\nStack Trace:\n%s\n", info->stack_trace);
    fprintf(f, "\nRegisters:\n%s\n", info->registers);
    fprintf(f, "\nMemory Map:\n%s\n", info->memory_map);
    fprintf(f, "\nTest Case: %s\n", info->testcase_path);
    fprintf(f, "Crash Log: %s\n", info->crash_log_path);
    fprintf(f, "iOS Version: %s\n", info->ios_version);

    fclose(f);
    return 0;
}

// Analyze crash and generate report
int analyze_crash(const char *log_path, const char *testcase_path) {
    if (!log_path || !testcase_path) {
        return -1;
    }

    crash_info_t info;
    memset(&info, 0, sizeof(info));

    // Get crash type
    info.type = analyze_crash_log(log_path);

    // Extract information
    info.stack_trace = extract_stack_trace(log_path);
    info.registers = extract_registers(log_path);
    info.timestamp = time(NULL);
    info.testcase_path = strdup(testcase_path);
    info.crash_log_path = strdup(log_path);
    info.ios_version = strdup("iOS 18.0");  // Set iOS version

    // Generate description based on crash type
    switch (info.type) {
        case CRASH_TYPE_NULL_PTR:
            info.description = "Null pointer dereference";
            break;
        case CRASH_TYPE_USE_AFTER_FREE:
            info.description = "Use-after-free";
            break;
        case CRASH_TYPE_DOUBLE_FREE:
            info.description = "Double free";
            break;
        case CRASH_TYPE_BUFFER_OVERFLOW:
            info.description = "Buffer overflow";
            break;
        case CRASH_TYPE_INTEGER_OVERFLOW:
            info.description = "Integer overflow";
            break;
        case CRASH_TYPE_TYPE_CONFUSION:
            info.description = "Type confusion";
            break;
        case CRASH_TYPE_RACE_CONDITION:
            info.description = "Race condition";
            break;
        case CRASH_TYPE_KERNEL_PANIC:
            info.description = "Kernel panic";
            break;
        case CRASH_TYPE_MEMORY_CORRUPTION:
            info.description = "Memory corruption";
            break;
        default:
            info.description = "Unknown crash type";
    }

    // Generate report
    int ret = generate_crash_report(&info);

    // Clean up
    free(info.stack_trace);
    free(info.registers);
    free(info.testcase_path);
    free(info.crash_log_path);
    free(info.ios_version);

    return ret;
} 