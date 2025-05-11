#ifndef FUZZKRIEG_PARALLEL_H
#define FUZZKRIEG_PARALLEL_H

#include "fuzzkrieg.h"

// Initialize parallel fuzzing with specified number of worker processes
int parallel_fuzzer_init(int num_worker_processes);

// Start worker processes
int start_workers(void);

// Stop worker processes
int stop_workers(void);

// Clean up parallel fuzzer resources
int parallel_fuzzer_cleanup(void);

// Get combined coverage information from all workers
int get_combined_coverage(coverage_info_t *coverage);

#endif // FUZZKRIEG_PARALLEL_H 