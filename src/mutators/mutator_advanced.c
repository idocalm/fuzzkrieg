#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../../include/fuzzkrieg.h"
#include "../../include/mutator_advanced.h"

// Mutate kernel structure
void mutate_kernel_struct(testcase_t *tc, const kernel_struct_template_t *template) {
    if (!tc || !tc->data || !template) {
        return;
    }

    // Ensure test case is large enough
    if (tc->size < template->size) {
        return;
    }

    // Copy template
    memcpy(tc->data, template->template, template->size);

    // Mutate variable fields
    for (size_t i = 0; i < template->variable_fields; i++) {
        size_t offset = (i * 4) % template->size;
        uint32_t value = kernel_interesting_32[rand() % (sizeof(kernel_interesting_32) / sizeof(kernel_interesting_32[0]))];
        memcpy(tc->data + offset, &value, sizeof(value));
    }
}

// Mutate memory access patterns
void mutate_memory_pattern(testcase_t *tc) {
    if (!tc || !tc->data) {
        return;
    }

    // Common memory access patterns
    static const uint8_t patterns[][8] = {
        {0x48, 0x8B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // mov rax, [rax]
        {0x48, 0x89, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // mov [rax], rcx
        {0x48, 0x8D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // lea rax, [rax]
        {0x48, 0x83, 0xEC, 0x00, 0x00, 0x00, 0x00, 0x00}  // sub rsp, imm8
    };

    // Select a random pattern
    const uint8_t *pattern = patterns[rand() % (sizeof(patterns) / sizeof(patterns[0]))];
    
    // Find a suitable location in the test case
    size_t pos = rand() % (tc->size - 8);
    memcpy(tc->data + pos, pattern, 8);
}

// Mutate system call parameters
void mutate_syscall(testcase_t *tc) {
    if (!tc || !tc->data) {
        return;
    }

    // Common syscall patterns
    static const uint8_t syscall_pattern[] = {
        0x48, 0x89, 0xC7,       // mov rdi, rax
        0x48, 0x89, 0xD6,       // mov rsi, rdx
        0x48, 0x89, 0xCA,       // mov rdx, rcx
        0x48, 0x89, 0xD9,       // mov rcx, rbx
        0x0F, 0x05              // syscall
    };

    // Find a suitable location
    if (tc->size >= sizeof(syscall_pattern)) {
        size_t pos = rand() % (tc->size - sizeof(syscall_pattern));
        memcpy(tc->data + pos, syscall_pattern, sizeof(syscall_pattern));
    }
}

// Mutate IOCTL commands
void mutate_ioctl(testcase_t *tc) {
    if (!tc || !tc->data) {
        return;
    }

    // Common IOCTL patterns
    static const uint8_t ioctl_pattern[] = {
        0x48, 0x89, 0xC7,       // mov rdi, rax
        0x48, 0x89, 0xD6,       // mov rsi, rdx
        0x48, 0x89, 0xCA,       // mov rdx, rcx
        0x48, 0x89, 0xD9,       // mov rcx, rbx
        0xE8, 0x00, 0x00, 0x00, 0x00  // call ioctl
    };

    // Find a suitable location
    if (tc->size >= sizeof(ioctl_pattern)) {
        size_t pos = rand() % (tc->size - sizeof(ioctl_pattern));
        memcpy(tc->data + pos, ioctl_pattern, sizeof(ioctl_pattern));
    }
}

// Mutate Mach message structures
void mutate_mach_msg(testcase_t *tc) {
    if (!tc || !tc->data) {
        return;
    }

    // Mach message header structure
    static const uint8_t mach_msg_header[] = {
        0x00, 0x00, 0x00, 0x00,  // msgh_bits
        0x00, 0x00, 0x00, 0x00,  // msgh_size
        0x00, 0x00, 0x00, 0x00,  // msgh_remote_port
        0x00, 0x00, 0x00, 0x00,  // msgh_local_port
        0x00, 0x00, 0x00, 0x00,  // msgh_voucher_port
        0x00, 0x00, 0x00, 0x00   // msgh_id
    };

    // Find a suitable location
    if (tc->size >= sizeof(mach_msg_header)) {
        size_t pos = rand() % (tc->size - sizeof(mach_msg_header));
        memcpy(tc->data + pos, mach_msg_header, sizeof(mach_msg_header));
    }
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