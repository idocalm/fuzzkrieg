#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../../include/fuzzkrieg.h"
#include "../../include/crash_analyzer.h"

#define MIN_CHUNK_SIZE 16
#define MAX_ITERATIONS 1000

// Test case chunk structure
typedef struct {
    size_t offset;
    size_t size;
    int is_essential;
} chunk_t;

// Initialize test case minimizer
int minimizer_init(void) {
    return 0;
}

// Check if test case still triggers the crash
int check_crash_reproducible(const char *testcase_path, const char *original_crash_log) {
    if (!testcase_path || !original_crash_log) {
        return 0;
    }

    // Run the test case and compare crash logs
    char cmd[1024];
    char temp_log[256];
    snprintf(temp_log, sizeof(temp_log), "/tmp/fuzzkrieg_minimizer_%d.log", getpid());
    
    // Run test case and capture crash log
    snprintf(cmd, sizeof(cmd), "./fuzzkrieg --testcase %s --log %s", testcase_path, temp_log);
    system(cmd);

    // Compare crash logs
    FILE *f1 = fopen(original_crash_log, "r");
    FILE *f2 = fopen(temp_log, "r");
    
    if (!f1 || !f2) {
        if (f1) fclose(f1);
        if (f2) fclose(f2);
        unlink(temp_log);
        return 0;
    }

    char line1[1024], line2[1024];
    int is_similar = 1;

    while (fgets(line1, sizeof(line1), f1) && fgets(line2, sizeof(line2), f2)) {
        if (strcmp(line1, line2) != 0) {
            is_similar = 0;
            break;
        }
    }

    fclose(f1);
    fclose(f2);
    unlink(temp_log);
    return is_similar;
}

// Split test case into chunks
chunk_t *split_into_chunks(const char *testcase_path, size_t *num_chunks) {
    if (!testcase_path || !num_chunks) {
        return NULL;
    }

    FILE *f = fopen(testcase_path, "rb");
    if (!f) {
        return NULL;
    }

    // Get file size
    fseek(f, 0, SEEK_END);
    size_t file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    // Calculate number of chunks
    *num_chunks = (file_size + MIN_CHUNK_SIZE - 1) / MIN_CHUNK_SIZE;
    chunk_t *chunks = malloc(*num_chunks * sizeof(chunk_t));
    
    if (!chunks) {
        fclose(f);
        return NULL;
    }

    // Initialize chunks
    for (size_t i = 0; i < *num_chunks; i++) {
        chunks[i].offset = i * MIN_CHUNK_SIZE;
        chunks[i].size = (i == *num_chunks - 1) ? 
            (file_size - i * MIN_CHUNK_SIZE) : MIN_CHUNK_SIZE;
        chunks[i].is_essential = 0;
    }

    fclose(f);
    return chunks;
}

// Create minimized test case
int create_minimized_testcase(const char *original_path, const char *minimized_path, 
                            chunk_t *chunks, size_t num_chunks) {
    if (!original_path || !minimized_path || !chunks) {
        return -1;
    }

    FILE *f_orig = fopen(original_path, "rb");
    FILE *f_min = fopen(minimized_path, "wb");
    
    if (!f_orig || !f_min) {
        if (f_orig) fclose(f_orig);
        if (f_min) fclose(f_min);
        return -1;
    }

    // Copy only essential chunks
    for (size_t i = 0; i < num_chunks; i++) {
        if (chunks[i].is_essential) {
            fseek(f_orig, chunks[i].offset, SEEK_SET);
            char *buffer = malloc(chunks[i].size);
            if (!buffer) {
                fclose(f_orig);
                fclose(f_min);
                return -1;
            }
            fread(buffer, 1, chunks[i].size, f_orig);
            fwrite(buffer, 1, chunks[i].size, f_min);
            free(buffer);
        }
    }

    fclose(f_orig);
    fclose(f_min);
    return 0;
}

// Minimize test case
int minimize_testcase(const char *testcase_path, const char *crash_log_path) {
    if (!testcase_path || !crash_log_path) {
        return -1;
    }

    // Split test case into chunks
    size_t num_chunks;
    chunk_t *chunks = split_into_chunks(testcase_path, &num_chunks);
    if (!chunks) {
        return -1;
    }

    // Create temporary file for minimized test case
    char temp_path[256];
    snprintf(temp_path, sizeof(temp_path), "/tmp/fuzzkrieg_minimized_%d", getpid());

    // Try to remove chunks one by one
    for (size_t i = 0; i < num_chunks; i++) {
        // Mark chunk as non-essential
        chunks[i].is_essential = 0;

        // Create minimized test case
        if (create_minimized_testcase(testcase_path, temp_path, chunks, num_chunks) != 0) {
            chunks[i].is_essential = 1;
            continue;
        }

        // Check if crash is still reproducible
        if (!check_crash_reproducible(temp_path, crash_log_path)) {
            // If crash is not reproducible, mark chunk as essential
            chunks[i].is_essential = 1;
        }
    }

    // Create final minimized test case
    char minimized_path[256];
    snprintf(minimized_path, sizeof(minimized_path), "crashes/testcases/minimized_%s", 
             strrchr(testcase_path, '/') + 1);
    
    int ret = create_minimized_testcase(testcase_path, minimized_path, chunks, num_chunks);

    // Clean up
    free(chunks);
    unlink(temp_path);

    return ret;
} 