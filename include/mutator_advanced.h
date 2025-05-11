#ifndef MUTATOR_ADVANCED_H
#define MUTATOR_ADVANCED_H

#include <stdint.h>

// Advanced mutation patterns
typedef struct {
    const char *name;
    const uint8_t *pattern;
    size_t length;
} mutation_pattern_t;

// Kernel-specific interesting values for iOS 18
static const uint32_t kernel_interesting_32[] = {
    // Memory addresses
    0x00000000, 0xffffffff, 0x80000000, 0x7fffffff,
    0x00001000, 0x00002000, 0x00004000, 0x00008000,
    0x00010000, 0x00020000, 0x00040000, 0x00080000,
    0x00100000, 0x00200000, 0x00400000, 0x00800000,
    
    // iOS 18 specific page sizes
    0x00001000, 0x00002000, 0x00004000, 0x00008000,
    0x00010000, 0x00020000, 0x00040000, 0x00080000,
    
    // iOS 18 kernel structure flags
    0x00000001, 0x00000002, 0x00000004, 0x00000008,
    0x00000010, 0x00000020, 0x00000040, 0x00000080,
    
    // iOS 18 specific values
    0xdeadbeef, 0xcafebabe, 0xbaadf00d, 0xfeedface,
    0x0000dead, 0x0000beef, 0xdead0000, 0xbeef0000,
    
    // iOS 18 alignment values
    0x00000007, 0x0000000f, 0x0000001f, 0x0000003f,
    0x0000007f, 0x000000ff, 0x000001ff, 0x000003ff,
    
    // iOS 18 permission bits
    0x00000001, 0x00000002, 0x00000004, 0x00000008,
    0x00000010, 0x00000020, 0x00000040, 0x00000080,
    
    // iOS 18 common offsets
    0x00000010, 0x00000020, 0x00000040, 0x00000080,
    0x00000100, 0x00000200, 0x00000400, 0x00000800,
    
    // iOS 18 stack-related
    0x00001000, 0x00002000, 0x00004000, 0x00008000,
    0x00010000, 0x00020000, 0x00040000, 0x00080000,
    
    // iOS 18 heap-related
    0x00000008, 0x00000010, 0x00000020, 0x00000040,
    0x00000080, 0x00000100, 0x00000200, 0x00000400
};

// iOS 18 specific patterns
static const mutation_pattern_t ios_patterns[] = {
    // iOS 18 kernel patterns
    {
        "ios18_kernel",
        (const uint8_t[]){0x55, 0x48, 0x89, 0xE5, 0x41, 0x57, 0x41, 0x56, 0x41, 0x55},
        10
    },
    // iOS 18 Mach message patterns
    {
        "ios18_mach_msg",
        (const uint8_t[]){0x48, 0x83, 0xEC, 0x28, 0x48, 0x89, 0x5C, 0x24, 0x20, 0x48},
        10
    },
    // iOS 18 syscall patterns
    {
        "ios18_syscall",
        (const uint8_t[]){0x48, 0x89, 0xC7, 0x48, 0x89, 0xD6, 0x48, 0x89, 0xCA, 0x0F},
        10
    }
};

// Advanced mutation strategies
typedef enum {
    MUTATE_KERNEL_STRUCT,    // Mutate kernel structure layouts
    MUTATE_MEMORY_PATTERN,   // Mutate memory access patterns
    MUTATE_SYSCALL,         // Mutate system call parameters
    MUTATE_IOCTL,           // Mutate IOCTL commands
    MUTATE_MACH_MSG,        // Mutate Mach message structures
    MUTATE_VM_OPERATION,    // Mutate VM operations
    MUTATE_TASK_OPERATION,  // Mutate task operations
    MUTATE_THREAD_OPERATION // Mutate thread operations
} advanced_mutation_strategy_t;

// iOS 18 kernel structure templates
typedef struct {
    const char *name;
    const uint8_t *template;
    size_t size;
    size_t variable_fields;
} kernel_struct_template_t;

// iOS 18 specific kernel structures
static const kernel_struct_template_t kernel_structs[] = {
    {
        "task",
        (const uint8_t[]){
            0x00, 0x00, 0x00, 0x00,  // task_ref_count
            0x00, 0x00, 0x00, 0x00,  // task_flags
            0x00, 0x00, 0x00, 0x00,  // task_ledger
            0x00, 0x00, 0x00, 0x00   // task_map
        },
        16,
        4
    },
    {
        "thread",
        (const uint8_t[]){
            0x00, 0x00, 0x00, 0x00,  // thread_ref_count
            0x00, 0x00, 0x00, 0x00,  // thread_state
            0x00, 0x00, 0x00, 0x00,  // thread_ledger
            0x00, 0x00, 0x00, 0x00   // thread_task
        },
        16,
        4
    },
    {
        "vm_map",
        (const uint8_t[]){
            0x00, 0x00, 0x00, 0x00,  // map_ref_count
            0x00, 0x00, 0x00, 0x00,  // map_flags
            0x00, 0x00, 0x00, 0x00,  // map_size
            0x00, 0x00, 0x00, 0x00   // map_pmap
        },
        16,
        4
    }
};

// Function declarations
void mutate_kernel_struct(testcase_t *tc, const kernel_struct_template_t *template);
void mutate_memory_pattern(testcase_t *tc);
void mutate_syscall(testcase_t *tc);
void mutate_ioctl(testcase_t *tc);
void mutate_mach_msg(testcase_t *tc);
void mutate_vm_operation(testcase_t *tc);
void mutate_task_operation(testcase_t *tc);
void mutate_thread_operation(testcase_t *tc);

#endif // MUTATOR_ADVANCED_H 