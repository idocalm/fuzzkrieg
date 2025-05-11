#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "../../include/fuzzkrieg.h"
#include "../../include/crash_analyzer.h"
#include "../../include/minimizer.h"

#define MAX_WORKERS 16
#define SHARED_MEMORY_SIZE (1024 * 1024)  // 1MB

// Worker process structure
typedef struct {
    pid_t pid;
    int worker_id;
    char *testcase_dir;
    char *coverage_dir;
    int is_running;
} worker_t;

// Shared memory structure for coverage information
typedef struct {
    uint8_t coverage_map[SHARED_MEMORY_SIZE];
    pthread_mutex_t mutex;
} shared_memory_t;

static worker_t workers[MAX_WORKERS];
static shared_memory_t *shared_mem = NULL;
static int num_workers = 0;

// Initialize parallel fuzzing
int parallel_fuzzer_init(int num_worker_processes) {
    if (num_worker_processes <= 0 || num_worker_processes > MAX_WORKERS) {
        return -1;
    }

    num_workers = num_worker_processes;

    // Create shared memory for coverage information
    shared_mem = mmap(NULL, sizeof(shared_memory_t), 
                     PROT_READ | PROT_WRITE, 
                     MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    
    if (shared_mem == MAP_FAILED) {
        return -1;
    }

    // Initialize shared memory
    memset(shared_mem->coverage_map, 0, SHARED_MEMORY_SIZE);
    pthread_mutex_init(&shared_mem->mutex, NULL);

    // Initialize worker structures
    for (int i = 0; i < num_workers; i++) {
        workers[i].worker_id = i;
        workers[i].is_running = 0;
        
        // Create worker-specific directories
        char dir[256];
        snprintf(dir, sizeof(dir), "testcases/worker_%d", i);
        mkdir(dir, 0755);
        workers[i].testcase_dir = strdup(dir);

        snprintf(dir, sizeof(dir), "coverage/worker_%d", i);
        mkdir(dir, 0755);
        workers[i].coverage_dir = strdup(dir);
    }

    return 0;
}

// Worker process function
void *worker_process(void *arg) {
    worker_t *worker = (worker_t *)arg;
    
    // Initialize fuzzer for this worker
    fuzzer_config_t config;
    memset(&config, 0, sizeof(config));
    
    config.testcase_dir = worker->testcase_dir;
    config.coverage_dir = worker->coverage_dir;
    config.worker_id = worker->worker_id;
    
    // Start fuzzing
    while (1) {
        // Generate and run test case
        testcase_t *testcase = generate_testcase();
        if (!testcase) {
            continue;
        }

        // Run test case and collect coverage
        coverage_info_t coverage;
        if (run_testcase(testcase, &coverage) == 0) {
            // Update shared coverage information
            pthread_mutex_lock(&shared_mem->mutex);
            for (size_t i = 0; i < coverage.num_edges; i++) {
                shared_mem->coverage_map[coverage.edges[i]] = 1;
            }
            pthread_mutex_unlock(&shared_mem->mutex);
        }

        // Check for crashes
        if (testcase->crashed) {
            char crash_log[256];
            snprintf(crash_log, sizeof(crash_log), 
                    "crashes/logs/crash_worker_%d_%llu.log",
                    worker->worker_id, (unsigned long long)time(NULL));
            
            // Save crash log
            save_crash_log(crash_log);
            
            // Analyze crash
            analyze_crash(crash_log, testcase->path);
            
            // Minimize test case
            minimize_testcase(testcase->path, crash_log);
        }

        // Clean up test case
        free_testcase(testcase);
    }

    return NULL;
}

// Start worker processes
int start_workers(void) {
    for (int i = 0; i < num_workers; i++) {
        pid_t pid = fork();
        
        if (pid < 0) {
            // Fork failed
            return -1;
        } else if (pid == 0) {
            // Child process
            worker_process(&workers[i]);
            exit(0);
        } else {
            // Parent process
            workers[i].pid = pid;
            workers[i].is_running = 1;
        }
    }

    return 0;
}

// Stop worker processes
int stop_workers(void) {
    for (int i = 0; i < num_workers; i++) {
        if (workers[i].is_running) {
            kill(workers[i].pid, SIGTERM);
            waitpid(workers[i].pid, NULL, 0);
            workers[i].is_running = 0;
        }
    }

    return 0;
}

// Clean up parallel fuzzer
int parallel_fuzzer_cleanup(void) {
    // Stop all workers
    stop_workers();

    // Clean up shared memory
    if (shared_mem) {
        pthread_mutex_destroy(&shared_mem->mutex);
        munmap(shared_mem, sizeof(shared_memory_t));
        shared_mem = NULL;
    }

    // Clean up worker structures
    for (int i = 0; i < num_workers; i++) {
        free(workers[i].testcase_dir);
        free(workers[i].coverage_dir);
    }

    return 0;
}

// Get coverage information from all workers
int get_combined_coverage(coverage_info_t *coverage) {
    if (!coverage || !shared_mem) {
        return -1;
    }

    pthread_mutex_lock(&shared_mem->mutex);
    
    // Count number of covered edges
    size_t num_edges = 0;
    for (size_t i = 0; i < SHARED_MEMORY_SIZE; i++) {
        if (shared_mem->coverage_map[i]) {
            num_edges++;
        }
    }

    // Allocate memory for edges
    coverage->edges = malloc(num_edges * sizeof(size_t));
    if (!coverage->edges) {
        pthread_mutex_unlock(&shared_mem->mutex);
        return -1;
    }

    // Copy covered edges
    size_t edge_idx = 0;
    for (size_t i = 0; i < SHARED_MEMORY_SIZE; i++) {
        if (shared_mem->coverage_map[i]) {
            coverage->edges[edge_idx++] = i;
        }
    }

    coverage->num_edges = num_edges;
    pthread_mutex_unlock(&shared_mem->mutex);

    return 0;
} 