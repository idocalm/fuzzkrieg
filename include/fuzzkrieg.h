#ifndef FUZZKRIEG_H
#define FUZZKRIEG_H

#include <stdint.h>
#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/lockdown.h>

// Maximum size for test cases
#define MAX_TESTCASE_SIZE (1024 * 1024)  // 1MB
#define COVERAGE_MAP_SIZE (1 << 16)      // 64KB coverage map

// Fuzzer states
typedef enum {
    FUZZ_STATE_INIT,
    FUZZ_STATE_RUNNING,
    FUZZ_STATE_PAUSED,
    FUZZ_STATE_CRASHED,
    FUZZ_STATE_ERROR
} fuzz_state_t;

// Test case structure
typedef struct {
    uint8_t *data;
    size_t size;
    uint32_t hash;
    uint64_t exec_time;
    uint32_t coverage_count;
} testcase_t;

// Coverage tracking structure
typedef struct {
    uint8_t *map;
    size_t map_size;
    uint32_t unique_paths;
    uint32_t total_hits;
} coverage_t;

// Device context structure
typedef struct {
    idevice_t device;
    lockdownd_client_t client;
    char *udid;
    char *product_type;
    char *product_version;
} device_ctx_t;

// Fuzzer configuration
typedef struct {
    char *target;
    char *output_dir;
    uint32_t max_iterations;
    uint32_t timeout;
    uint32_t max_crashes;
    uint8_t verbose;
} fuzz_config_t;

// Core fuzzer structure
typedef struct {
    fuzz_state_t state;
    fuzz_config_t config;
    device_ctx_t device;
    coverage_t coverage;
    testcase_t *testcases;
    uint32_t testcase_count;
    uint32_t crash_count;
    uint64_t start_time;
} fuzzer_t;

// Function declarations
int fuzzer_init(fuzzer_t *fuzzer, fuzz_config_t *config);
int fuzzer_run(fuzzer_t *fuzzer);
void fuzzer_cleanup(fuzzer_t *fuzzer);

// Device management
int device_connect(device_ctx_t *ctx, const char *udid);
int device_disconnect(device_ctx_t *ctx);

// Coverage tracking
int coverage_init(coverage_t *coverage);
int coverage_update(coverage_t *coverage, const uint8_t *map, size_t size);
void coverage_cleanup(coverage_t *coverage);

// Test case management
testcase_t *generate_testcase(fuzzer_t *fuzzer);
int execute_testcase(fuzzer_t *fuzzer, testcase_t *tc);
int update_coverage(fuzzer_t *fuzzer, testcase_t *tc);
int check_crash(fuzzer_t *fuzzer);
void handle_crash(fuzzer_t *fuzzer, testcase_t *tc);
int is_interesting(fuzzer_t *fuzzer, testcase_t *tc);
void save_interesting_case(fuzzer_t *fuzzer, testcase_t *tc);

#endif // FUZZKRIEG_H 