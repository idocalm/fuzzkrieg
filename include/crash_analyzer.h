#ifndef FUZZKRIEG_CRASH_ANALYZER_H
#define FUZZKRIEG_CRASH_ANALYZER_H

#include <stdint.h>

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
int crash_analyzer_init(void);

// Analyze crash log and determine crash type
crash_type_t analyze_crash_log(const char *log_path);

// Extract stack trace from crash log
char *extract_stack_trace(const char *log_path);

// Extract register values from crash log
char *extract_registers(const char *log_path);

// Generate crash report
int generate_crash_report(crash_info_t *info);

// Analyze crash and generate report
int analyze_crash(const char *log_path, const char *testcase_path);

#endif // FUZZKRIEG_CRASH_ANALYZER_H 