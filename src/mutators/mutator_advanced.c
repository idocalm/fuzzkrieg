#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../../include/fuzzkrieg.h"
#include "../../include/mutator_advanced.h"

// Kernel structure templates
kernel_struct_t kernel_structs[NUM_KERNEL_STRUCTS] = {
    {
        .name = "task",
        .size = sizeof(struct task),
        .template = NULL  // Will be initialized with actual template
    },
    {
        .name = "thread",
        .size = sizeof(struct thread),
        .template = NULL
    },
    {
        .name = "vm_map",
        .size = sizeof(struct vm_map),
        .template = NULL
    },
    {
        .name = "mach_msg_header",
        .size = sizeof(struct mach_msg_header),
        .template = NULL
    },
    {
        .name = "ioctl_command",
        .size = sizeof(struct ioctl_command),
        .template = NULL
    }
};

// Mutate kernel structure
int mutate_kernel_struct(testcase_t *tc, const kernel_struct_t *struct_template) {
    if (!tc || !struct_template || !struct_template->template) {
        return -1;
    }

    // Allocate space for the structure
    size_t new_size = tc->size + struct_template->size;
    uint8_t *new_data = realloc(tc->data, new_size);
    if (!new_data) {
        return -1;
    }

    // Copy template and mutate variable fields
    memcpy(new_data + tc->size, struct_template->template, struct_template->size);
    
    // Mutate some fields randomly
    for (size_t i = 0; i < struct_template->size; i += sizeof(uint32_t)) {
        if (rand() % 2) {  // 50% chance to mutate each field
            *(uint32_t*)(new_data + tc->size + i) = rand();
        }
    }

    tc->data = new_data;
    tc->size = new_size;
    return 0;
}

// Mutate memory pattern
int mutate_memory_pattern(testcase_t *tc) {
    if (!tc) {
        return -1;
    }

    // Insert random memory patterns
    size_t pattern_size = 16 + (rand() % 48);  // 16-64 bytes
    size_t new_size = tc->size + pattern_size;
    uint8_t *new_data = realloc(tc->data, new_size);
    if (!new_data) {
        return -1;
    }

    // Generate pattern
    for (size_t i = 0; i < pattern_size; i++) {
        new_data[tc->size + i] = rand() % 256;
    }

    tc->data = new_data;
    tc->size = new_size;
    return 0;
}

// Mutate syscall
int mutate_syscall(testcase_t *tc) {
    if (!tc) {
        return -1;
    }

    // Insert syscall pattern
    const char *syscall_pattern = "\x00\x00\x00\x00\x00\x00\x00\x00";  // Example pattern
    size_t pattern_size = strlen(syscall_pattern);
    size_t new_size = tc->size + pattern_size;
    uint8_t *new_data = realloc(tc->data, new_size);
    if (!new_data) {
        return -1;
    }

    // Insert at random position
    size_t pos = rand() % (tc->size + 1);
    memmove(new_data + pos + pattern_size, new_data + pos, tc->size - pos);
    memcpy(new_data + pos, syscall_pattern, pattern_size);

    tc->data = new_data;
    tc->size = new_size;
    return 0;
}

// Mutate IOCTL
int mutate_ioctl(testcase_t *tc) {
    if (!tc) {
        return -1;
    }

    // Insert IOCTL pattern
    const char *ioctl_pattern = "\x00\x00\x00\x00\x00\x00\x00\x00";  // Example pattern
    size_t pattern_size = strlen(ioctl_pattern);
    size_t new_size = tc->size + pattern_size;
    uint8_t *new_data = realloc(tc->data, new_size);
    if (!new_data) {
        return -1;
    }

    // Insert at random position
    size_t pos = rand() % (tc->size + 1);
    memmove(new_data + pos + pattern_size, new_data + pos, tc->size - pos);
    memcpy(new_data + pos, ioctl_pattern, pattern_size);

    tc->data = new_data;
    tc->size = new_size;
    return 0;
}

// Mutate Mach message
int mutate_mach_msg(testcase_t *tc) {
    if (!tc) {
        return -1;
    }

    // Insert Mach message header
    const char *mach_header = "\x00\x00\x00\x00\x00\x00\x00\x00";  // Example header
    size_t header_size = strlen(mach_header);
    size_t new_size = tc->size + header_size;
    uint8_t *new_data = realloc(tc->data, new_size);
    if (!new_data) {
        return -1;
    }

    // Insert at random position
    size_t pos = rand() % (tc->size + 1);
    memmove(new_data + pos + header_size, new_data + pos, tc->size - pos);
    memcpy(new_data + pos, mach_header, header_size);

    tc->data = new_data;
    tc->size = new_size;
    return 0;
}

// Mutate VM operations
void mutate_vm_operation(testcase_t *tc) {
    if (!tc || !tc->data) {
        return;
    }

    // Common VM operation patterns
    static const uint8_t vm_patterns[][16] = {
        // vm_allocate
        {0x48, 0x89, 0xC7, 0x48, 0x89, 0xD6, 0x48, 0x89, 0xCA, 0x48, 0x89, 0xD9, 0xE8, 0x00, 0x00, 0x00},
        // vm_deallocate
        {0x48, 0x89, 0xC7, 0x48, 0x89, 0xD6, 0x48, 0x89, 0xCA, 0x48, 0x89, 0xD9, 0xE8, 0x00, 0x00, 0x00},
        // vm_protect
        {0x48, 0x89, 0xC7, 0x48, 0x89, 0xD6, 0x48, 0x89, 0xCA, 0x48, 0x89, 0xD9, 0xE8, 0x00, 0x00, 0x00}
    };

    // Select a random pattern
    const uint8_t *pattern = vm_patterns[rand() % (sizeof(vm_patterns) / sizeof(vm_patterns[0]))];
    
    // Find a suitable location
    if (tc->size >= sizeof(vm_patterns[0])) {
        size_t pos = rand() % (tc->size - sizeof(vm_patterns[0]));
        memcpy(tc->data + pos, pattern, sizeof(vm_patterns[0]));
    }
}

// Mutate task operations
void mutate_task_operation(testcase_t *tc) {
    if (!tc || !tc->data) {
        return;
    }

    // Common task operation patterns
    static const uint8_t task_patterns[][16] = {
        // task_create
        {0x48, 0x89, 0xC7, 0x48, 0x89, 0xD6, 0x48, 0x89, 0xCA, 0x48, 0x89, 0xD9, 0xE8, 0x00, 0x00, 0x00},
        // task_terminate
        {0x48, 0x89, 0xC7, 0x48, 0x89, 0xD6, 0x48, 0x89, 0xCA, 0x48, 0x89, 0xD9, 0xE8, 0x00, 0x00, 0x00},
        // task_suspend
        {0x48, 0x89, 0xC7, 0x48, 0x89, 0xD6, 0x48, 0x89, 0xCA, 0x48, 0x89, 0xD9, 0xE8, 0x00, 0x00, 0x00}
    };

    // Select a random pattern
    const uint8_t *pattern = task_patterns[rand() % (sizeof(task_patterns) / sizeof(task_patterns[0]))];
    
    // Find a suitable location
    if (tc->size >= sizeof(task_patterns[0])) {
        size_t pos = rand() % (tc->size - sizeof(task_patterns[0]));
        memcpy(tc->data + pos, pattern, sizeof(task_patterns[0]));
    }
}

// Mutate thread operations
void mutate_thread_operation(testcase_t *tc) {
    if (!tc || !tc->data) {
        return;
    }

    // Common thread operation patterns
    static const uint8_t thread_patterns[][16] = {
        // thread_create
        {0x48, 0x89, 0xC7, 0x48, 0x89, 0xD6, 0x48, 0x89, 0xCA, 0x48, 0x89, 0xD9, 0xE8, 0x00, 0x00, 0x00},
        // thread_terminate
        {0x48, 0x89, 0xC7, 0x48, 0x89, 0xD6, 0x48, 0x89, 0xCA, 0x48, 0x89, 0xD9, 0xE8, 0x00, 0x00, 0x00},
        // thread_suspend
        {0x48, 0x89, 0xC7, 0x48, 0x89, 0xD6, 0x48, 0x89, 0xCA, 0x48, 0x89, 0xD9, 0xE8, 0x00, 0x00, 0x00}
    };

    // Select a random pattern
    const uint8_t *pattern = thread_patterns[rand() % (sizeof(thread_patterns) / sizeof(thread_patterns[0]))];
    
    // Find a suitable location
    if (tc->size >= sizeof(thread_patterns[0])) {
        size_t pos = rand() % (tc->size - sizeof(thread_patterns[0]));
        memcpy(tc->data + pos, pattern, sizeof(thread_patterns[0]));
    }
}

// Advanced test case mutation
int testcase_mutate_advanced(testcase_t *tc) {
    if (!tc || !tc->data) {
        return -1;
    }

    // Select a random advanced mutation strategy
    advanced_mutation_strategy_t strategy = rand() % 8;

    switch (strategy) {
        case MUTATE_KERNEL_STRUCT:
            mutate_kernel_struct(tc, &kernel_structs[rand() % (sizeof(kernel_structs) / sizeof(kernel_structs[0]))]);
            break;

        case MUTATE_MEMORY_PATTERN:
            mutate_memory_pattern(tc);
            break;

        case MUTATE_SYSCALL:
            mutate_syscall(tc);
            break;

        case MUTATE_IOCTL:
            mutate_ioctl(tc);
            break;

        case MUTATE_MACH_MSG:
            mutate_mach_msg(tc);
            break;

        case MUTATE_VM_OPERATION:
            mutate_vm_operation(tc);
            break;

        case MUTATE_TASK_OPERATION:
            mutate_task_operation(tc);
            break;

        case MUTATE_THREAD_OPERATION:
            mutate_thread_operation(tc);
            break;
    }

    return 0;
} 