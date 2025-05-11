#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <signal.h>
#include "../../include/fuzzkrieg.h"

// Global fuzzer instance
static fuzzer_t g_fuzzer;

// Signal handler
static void signal_handler(int signum) {
    if (g_fuzzer.state == FUZZ_STATE_RUNNING) {
        printf("\nReceived signal %d, cleaning up...\n", signum);
        g_fuzzer.state = FUZZ_STATE_PAUSED;
        fuzzer_cleanup(&g_fuzzer);
    }
    exit(0);
}

// Print usage information
static void print_usage(const char *prog) {
    printf("Usage: %s [options]\n", prog);
    printf("Options:\n");
    printf("  -d, --device <udid>    Device UDID to target\n");
    printf("  -t, --target <path>    Target binary to fuzz\n");
    printf("  -o, --output <dir>     Output directory for results\n");
    printf("  -i, --iterations <n>   Maximum number of iterations\n");
    printf("  -T, --timeout <ms>     Timeout per test case (ms)\n");
    printf("  -v, --verbose         Enable verbose output\n");
    printf("  -h, --help            Show this help message\n");
}

int main(int argc, char *argv[]) {
    // Initialize fuzzer configuration
    fuzz_config_t config = {
        .target = NULL,
        .output_dir = "fuzz_results",
        .max_iterations = 1000000,
        .timeout = 1000,
        .max_crashes = 100,
        .verbose = 0
    };

    // Parse command line options
    static struct option long_options[] = {
        {"device", required_argument, 0, 'd'},
        {"target", required_argument, 0, 't'},
        {"output", required_argument, 0, 'o'},
        {"iterations", required_argument, 0, 'i'},
        {"timeout", required_argument, 0, 'T'},
        {"verbose", no_argument, 0, 'v'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    int opt;
    int option_index = 0;
    while ((opt = getopt_long(argc, argv, "d:t:o:i:T:vh", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'd':
                // Device UDID will be handled by device_connect
                break;
            case 't':
                config.target = strdup(optarg);
                break;
            case 'o':
                config.output_dir = strdup(optarg);
                break;
            case 'i':
                config.max_iterations = atoi(optarg);
                break;
            case 'T':
                config.timeout = atoi(optarg);
                break;
            case 'v':
                config.verbose = 1;
                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }

    // Validate required options
    if (!config.target) {
        fprintf(stderr, "Error: Target binary must be specified\n");
        print_usage(argv[0]);
        return 1;
    }

    // Set up signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    // Initialize fuzzer
    if (fuzzer_init(&g_fuzzer, &config) != 0) {
        fprintf(stderr, "Failed to initialize fuzzer\n");
        return 1;
    }

    // Print fuzzer information
    printf("Fuzzkrieg - iOS Kernel Fuzzer\n");
    printf("Target: %s\n", config.target);
    printf("Output directory: %s\n", config.output_dir);
    printf("Max iterations: %u\n", config.max_iterations);
    printf("Timeout: %u ms\n", config.timeout);
    printf("Device: %s\n", g_fuzzer.device.udid);
    printf("iOS version: %s\n", g_fuzzer.device.product_version);
    printf("\nStarting fuzzing...\n");

    // Run fuzzer
    int ret = fuzzer_run(&g_fuzzer);

    // Clean up
    fuzzer_cleanup(&g_fuzzer);
    free(config.target);
    free(config.output_dir);

    return ret;
} 