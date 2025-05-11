#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../../include/fuzzkrieg.h"

// Mutation strategies
typedef enum {
    MUTATE_BITFLIP,
    MUTATE_BYTE_FLIP,
    MUTATE_ARITHMETIC,
    MUTATE_INTERESTING,
    MUTATE_DICTIONARY,
    MUTATE_HAVOC
} mutation_strategy_t;

// Interesting values for mutation
static const uint8_t interesting_8[] = {
    0, 1, 2, 4, 8, 16, 32, 64, 128, 255,
    0x7f, 0x80, 0xff, 0xfe, 0xfd, 0xfb
};

static const uint16_t interesting_16[] = {
    0, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096,
    0x7fff, 0x8000, 0xffff, 0xfffe, 0xfffd, 0xfffb
};

// Initialize random number generator
static void init_random(void) {
    static int initialized = 0;
    if (!initialized) {
        srand(time(NULL));
        initialized = 1;
    }
}

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

    free(tc->data);
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

// Load a test case from file
testcase_t *testcase_load(const char *path) {
    if (!path) {
        return NULL;
    }

    FILE *f = fopen(path, "rb");
    if (!f) {
        return NULL;
    }

    // Get file size
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (size == 0 || size > MAX_TESTCASE_SIZE) {
        fclose(f);
        return NULL;
    }

    // Read file
    uint8_t *data = malloc(size);
    if (!data) {
        fclose(f);
        return NULL;
    }

    if (fread(data, 1, size, f) != size) {
        free(data);
        fclose(f);
        return NULL;
    }

    fclose(f);

    // Create test case
    testcase_t *tc = testcase_create(data, size);
    free(data);
    return tc;
}

// Mutate a test case
int testcase_mutate(testcase_t *tc) {
    if (!tc || !tc->data || tc->size == 0) {
        return -1;
    }

    init_random();

    // Select mutation strategy
    mutation_strategy_t strategy = rand() % 6;
    size_t pos = rand() % tc->size;

    switch (strategy) {
        case MUTATE_BITFLIP:
            // Flip a random bit
            tc->data[pos] ^= (1 << (rand() % 8));
            break;

        case MUTATE_BYTE_FLIP:
            // Flip a random byte
            tc->data[pos] = ~tc->data[pos];
            break;

        case MUTATE_ARITHMETIC:
            // Add or subtract a small value
            {
                int delta = (rand() % 16) - 8;
                tc->data[pos] += delta;
            }
            break;

        case MUTATE_INTERESTING:
            // Replace with an interesting value
            if (rand() % 2) {
                tc->data[pos] = interesting_8[rand() % (sizeof(interesting_8) / sizeof(interesting_8[0]))];
            } else if (pos + 1 < tc->size) {
                uint16_t val = interesting_16[rand() % (sizeof(interesting_16) / sizeof(interesting_16[0]))];
                tc->data[pos] = val & 0xff;
                tc->data[pos + 1] = (val >> 8) & 0xff;
            }
            break;

        case MUTATE_DICTIONARY:
            // TODO: Implement dictionary-based mutation
            break;

        case MUTATE_HAVOC:
            // Perform multiple random mutations
            {
                int num_mutations = 1 + (rand() % 4);
                for (int i = 0; i < num_mutations; i++) {
                    testcase_mutate(tc);
                }
            }
            break;
    }

    return 0;
}

// Generate a random test case
testcase_t *testcase_generate_random(size_t min_size, size_t max_size) {
    init_random();

    size_t size = min_size + (rand() % (max_size - min_size + 1));
    uint8_t *data = malloc(size);
    if (!data) {
        return NULL;
    }

    // Generate random data
    for (size_t i = 0; i < size; i++) {
        data[i] = rand() % 256;
    }

    testcase_t *tc = testcase_create(data, size);
    free(data);
    return tc;
}

// Calculate test case hash
uint32_t testcase_hash(testcase_t *tc) {
    if (!tc || !tc->data) {
        return 0;
    }

    uint32_t hash = 0;
    for (size_t i = 0; i < tc->size; i++) {
        hash = ((hash << 5) + hash) + tc->data[i];
    }

    return hash;
} 