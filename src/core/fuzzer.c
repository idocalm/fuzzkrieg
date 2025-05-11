#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "../../include/fuzzkrieg.h"

// Initialize the fuzzer with given configuration
int fuzzer_init(fuzzer_t *fuzzer, fuzz_config_t *config) {
    if (!fuzzer || !config) {
        return -1;
    }

    memset(fuzzer, 0, sizeof(fuzzer_t));
    memcpy(&fuzzer->config, config, sizeof(fuzz_config_t));
    
    // Initialize coverage tracking
    if (coverage_init(&fuzzer->coverage) != 0) {
        fprintf(stderr, "Failed to initialize coverage tracking\n");
        return -1;
    }

    // Connect to device
    if (device_connect(&fuzzer->device, NULL) != 0) {
        fprintf(stderr, "Failed to connect to device\n");
        coverage_cleanup(&fuzzer->coverage);
        return -1;
    }

    fuzzer->state = FUZZ_STATE_INIT;
    fuzzer->start_time = time(NULL);
    
    return 0;
}

// Main fuzzing loop
int fuzzer_run(fuzzer_t *fuzzer) {
    if (!fuzzer || fuzzer->state == FUZZ_STATE_ERROR) {
        return -1;
    }

    fuzzer->state = FUZZ_STATE_RUNNING;
    uint32_t iteration = 0;

    while (fuzzer->state == FUZZ_STATE_RUNNING) {
        // Check if we've reached max iterations
        if (iteration >= fuzzer->config.max_iterations) {
            break;
        }

        // Generate or mutate test case
        testcase_t *tc = generate_testcase(fuzzer);
        if (!tc) {
            fprintf(stderr, "Failed to generate test case\n");
            continue;
        }

        // Execute test case on device
        if (execute_testcase(fuzzer, tc) != 0) {
            fprintf(stderr, "Failed to execute test case\n");
            testcase_free(tc);
            continue;
        }

        // Update coverage information
        if (update_coverage(fuzzer, tc) != 0) {
            fprintf(stderr, "Failed to update coverage\n");
        }

        // Check for crashes
        if (check_crash(fuzzer) != 0) {
            fuzzer->state = FUZZ_STATE_CRASHED;
            handle_crash(fuzzer, tc);
            break;
        }

        // Save interesting test cases
        if (is_interesting(fuzzer, tc)) {
            save_interesting_case(fuzzer, tc);
        }

        testcase_free(tc);
        iteration++;
    }

    return 0;
}

// Clean up fuzzer resources
void fuzzer_cleanup(fuzzer_t *fuzzer) {
    if (!fuzzer) {
        return;
    }

    // Disconnect from device
    device_disconnect(&fuzzer->device);

    // Clean up coverage tracking
    coverage_cleanup(&fuzzer->coverage);

    // Free test cases
    for (uint32_t i = 0; i < fuzzer->testcase_count; i++) {
        testcase_free(&fuzzer->testcases[i]);
    }
    free(fuzzer->testcases);

    memset(fuzzer, 0, sizeof(fuzzer_t));
}

// Helper function to generate test cases
testcase_t *generate_testcase(fuzzer_t *fuzzer) {
    testcase_t *tc = malloc(sizeof(testcase_t));
    if (!tc) {
        return NULL;
    }

    // Generate a test case with a mix of strategies
    size_t size = 64 + (rand() % (MAX_TESTCASE_SIZE - 64));  // Minimum 64 bytes
    tc->data = malloc(size);
    if (!tc->data) {
        free(tc);
        return NULL;
    }
    tc->size = size;

    // Fill with initial pattern
    for (size_t i = 0; i < size; i++) {
        tc->data[i] = rand() % 256;
    }

    // Apply mutation strategies
    int strategy = rand() % 5;
    switch (strategy) {
        case 0:  // Kernel structure mutation
            mutate_kernel_struct(tc, &kernel_structs[rand() % NUM_KERNEL_STRUCTS]);
            break;
        case 1:  // Memory pattern mutation
            mutate_memory_pattern(tc);
            break;
        case 2:  // System call mutation
            mutate_syscall(tc);
            break;
        case 3:  // IOCTL mutation
            mutate_ioctl(tc);
            break;
        case 4:  // Mach message mutation
            mutate_mach_msg(tc);
            break;
    }

    tc->hash = 0;  // Will be computed when needed
    tc->exec_time = 0;
    tc->coverage_count = 0;

    return tc;
}

// Helper function to execute test case on device
int execute_testcase(fuzzer_t *fuzzer, testcase_t *tc) {
    if (!fuzzer || !tc || !tc->data) {
        return -1;
    }

    // Create a temporary file for the test case
    char temp_path[256];
    snprintf(temp_path, sizeof(temp_path), "/tmp/fuzzkrieg_%d", getpid());
    if (testcase_save(tc, temp_path) != 0) {
        return -1;
    }

    // Transfer test case to device
    if (device_transfer_file(&fuzzer->device, temp_path, "/var/root/testcase") != 0) {
        unlink(temp_path);
        return -1;
    }

    // Execute test case on device
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "chmod +x /var/root/testcase && /var/root/testcase");
    if (device_execute_command(&fuzzer->device, cmd) != 0) {
        unlink(temp_path);
        return -1;
    }

    // Collect coverage information
    if (device_collect_coverage(&fuzzer->device, &fuzzer->coverage) != 0) {
        unlink(temp_path);
        return -1;
    }

    unlink(temp_path);
    return 0;
}

// Helper function to update coverage information
int update_coverage(fuzzer_t *fuzzer, testcase_t *tc) {
    if (!fuzzer || !tc) {
        return -1;
    }

    // Update coverage map
    if (coverage_update(&fuzzer->coverage, fuzzer->coverage.map, fuzzer->coverage.map_size) != 0) {
        return -1;
    }

    // Update test case coverage count
    tc->coverage_count = 0;
    for (size_t i = 0; i < fuzzer->coverage.map_size; i++) {
        if (fuzzer->coverage.map[i]) {
            tc->coverage_count++;
        }
    }

    return 0;
}

// Helper function to check for crashes
int check_crash(fuzzer_t *fuzzer) {
    if (!fuzzer) {
        return -1;
    }

    // Check device status
    if (device_check_status(&fuzzer->device) != 0) {
        return 1;  // Device is in a crashed state
    }

    // Check for crash logs
    char crash_log[256];
    snprintf(crash_log, sizeof(crash_log), "/var/log/crash_%s.log", fuzzer->device.udid);
    if (device_file_exists(&fuzzer->device, crash_log)) {
        return 1;  // Crash log exists
    }

    return 0;  // No crash detected
}

// Helper function to handle crashes
void handle_crash(fuzzer_t *fuzzer, testcase_t *tc) {
    if (!fuzzer || !tc) {
        return;
    }

    fuzzer->crash_count++;
    
    // Save crash information
    char crash_path[256];
    snprintf(crash_path, sizeof(crash_path), "%s/crash_%u", 
             fuzzer->config.output_dir, fuzzer->crash_count);
    
    // Save test case
    testcase_save(tc, crash_path);

    // Save crash log
    char crash_log[256];
    snprintf(crash_log, sizeof(crash_log), "%s/crash_%u.log", 
             fuzzer->config.output_dir, fuzzer->crash_count);
    
    // Copy crash log from device
    device_copy_file(&fuzzer->device, "/var/log/crash.log", crash_log);

    // Analyze crash
    analyze_crash(crash_log, crash_path);

    // Minimize test case
    minimize_testcase(crash_path, crash_log);
}

// Helper function to determine if a test case is interesting
int is_interesting(fuzzer_t *fuzzer, testcase_t *tc) {
    if (!fuzzer || !tc) {
        return 0;
    }

    // Check if this test case increases coverage
    if (tc->coverage_count > fuzzer->coverage.unique_paths) {
        return 1;
    }

    // Check if this test case has unique execution time
    if (tc->exec_time > 0 && tc->exec_time < fuzzer->config.timeout) {
        return 1;
    }

    // Check if this test case has unique hash
    for (uint32_t i = 0; i < fuzzer->testcase_count; i++) {
        if (fuzzer->testcases[i].hash == tc->hash) {
            return 0;
        }
    }

    return 1;
}

// Helper function to save interesting test cases
void save_interesting_case(fuzzer_t *fuzzer, testcase_t *tc) {
    if (!fuzzer || !tc) {
        return;
    }

    char path[256];
    snprintf(path, sizeof(path), "%s/interesting_%u", 
             fuzzer->config.output_dir, fuzzer->testcase_count);
    
    if (testcase_save(tc, path) == 0) {
        // Add to test case array
        fuzzer->testcases = realloc(fuzzer->testcases, 
                                  (fuzzer->testcase_count + 1) * sizeof(testcase_t));
        if (fuzzer->testcases) {
            memcpy(&fuzzer->testcases[fuzzer->testcase_count], tc, sizeof(testcase_t));
            fuzzer->testcase_count++;
        }
    }
} 