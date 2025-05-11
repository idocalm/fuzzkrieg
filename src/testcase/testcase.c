#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/fuzzkrieg.h"

// Create a new test case
testcase_t *testcase_create(const uint8_t *data, size_t size) {
    if (!data || size == 0 || size > MAX_TESTCASE_SIZE) {
        return NULL;
    }

    testcase_t *tc = malloc(sizeof(testcase_t));
    if (!tc) {
        return NULL;
    }

    tc->data = malloc(size);
    if (!tc->data) {
        free(tc);
        return NULL;
    }

    memcpy(tc->data, data, size);
    tc->size = size;
    tc->hash = 0;
    tc->exec_time = 0;
    tc->coverage_count = 0;

    return tc;
}

// Free a test case
void testcase_free(testcase_t *tc) {
    if (!tc) {
        return;
    }

    if (tc->data) {
        free(tc->data);
    }
    free(tc);
}

// Save a test case to file
int testcase_save(testcase_t *tc, const char *path) {
    if (!tc || !tc->data || !path) {
        return -1;
    }

    FILE *f = fopen(path, "wb");
    if (!f) {
        return -1;
    }

    if (fwrite(tc->data, 1, tc->size, f) != tc->size) {
        fclose(f);
        return -1;
    }

    fclose(f);
    return 0;
} 