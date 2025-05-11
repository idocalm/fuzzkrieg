#ifndef FUZZKRIEG_MINIMIZER_H
#define FUZZKRIEG_MINIMIZER_H

// Initialize test case minimizer
int minimizer_init(void);

// Minimize test case while preserving crash reproduction
int minimize_testcase(const char *testcase_path, const char *crash_log_path);

#endif // FUZZKRIEG_MINIMIZER_H 