#include "../unit_tests.h"

#include <toolbox/memory_pool.h>

MU_TEST(memory_pool_test_int) {
    MemoryPool* pool = memory_pool_alloc(sizeof(int), 5);
    mu_assert_not_null(pool);

    int* objects[5] = {0};

    for(size_t i = 0; i != 5; ++i) {
        objects[i] = memory_pool_alloc_object(pool);
        mu_assert_not_null(objects[i]);
        *objects[i] = i;
    }

    mu_assert_null(memory_pool_alloc_object(pool));

    for(size_t i = 0; i != 5; ++i) {
        mu_assert_int_eq(i, *objects[i]);
    }

    for(size_t i = 0; i != 5; ++i) {
        memory_pool_free_object(pool, objects[(i * 3) % 5]);
    }

    for(size_t i = 0; i != 5; ++i) {
        objects[i] = memory_pool_alloc_object(pool);
        mu_assert_not_null(objects[i]);
        *objects[i] = i + 10;
    }
    mu_assert_null(memory_pool_alloc_object(pool));

    for(size_t i = 0; i != 5; ++i) {
        mu_assert_int_eq(i + 10, *objects[i]);
    }

    memory_pool_free(pool);
}

MU_TEST_SUITE(memory_pool_test_suite) {
    MU_RUN_TEST(memory_pool_test_int);
}

int run_minunit_memory_pool_test(void) {
    MU_RUN_SUITE(memory_pool_test_suite);
    return MU_EXIT_CODE;
}
