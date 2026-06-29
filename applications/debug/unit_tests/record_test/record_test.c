#include "../unit_tests.h"

#include <toolbox/api_lock.h>

#define RECORD_TEST "testtesttest"

#define SECRET_VALUE (0xd00d00caca)

#define SMALL_DELAY_TICK (20)
#define NUM_ITERATIONS   (50)

static int32_t record_consumer_thread(void* arg) {
    furi_assert(arg);
    FuriApiLock lock = arg;

    const uint64_t* data_ptr = furi_record_open_ex(RECORD_TEST, 0);
    api_lock_unlock(lock);

    int32_t success = (*data_ptr == SECRET_VALUE);

    furi_record_close(RECORD_TEST);
    return success;
}

MU_TEST(record_basic_test) {
    uint64_t data = SECRET_VALUE;
    uint64_t* data_ptr = NULL;

    mu_assert_null(furi_record_open_ex(RECORD_TEST, 0));
    mu_assert_null(furi_record_open_ex(RECORD_TEST, SMALL_DELAY_TICK));

    furi_record_create(RECORD_TEST, &data);
    mu_check(furi_record_exists(RECORD_TEST));

    data_ptr = furi_record_open_ex(RECORD_TEST, 0);
    mu_assert_not_null(data_ptr);

    mu_assert_pointers_eq(data_ptr, &data);
    mu_assert_mem_eq(data_ptr, &data, sizeof(data));

    furi_record_close(RECORD_TEST);

    furi_record_destroy(RECORD_TEST);
    mu_check(!furi_record_exists(RECORD_TEST));

    mu_assert_null(furi_record_open_ex(RECORD_TEST, 0));
    mu_assert_null(furi_record_open_ex(RECORD_TEST, SMALL_DELAY_TICK));
}

MU_TEST(record_ref_count_test) {
    uint64_t data = SECRET_VALUE;
    furi_record_create(RECORD_TEST, &data);

    for(uint32_t i = 0; i < NUM_ITERATIONS; ++i) {
        const uint64_t* data_ptr = furi_record_open(RECORD_TEST);
        mu_assert_pointers_eq(data_ptr, &data);
        mu_assert_mem_eq(data_ptr, &data, sizeof(data));
    }

    for(uint32_t i = 0; i < NUM_ITERATIONS; ++i) {
        furi_record_close(RECORD_TEST);
    }

    furi_record_destroy(RECORD_TEST);
}

MU_TEST(record_multithread_test) {
    uint64_t data = SECRET_VALUE;
    furi_record_create(RECORD_TEST, &data);

    FuriApiLock lock = api_lock_alloc_locked();
    FuriThread* consumer_thread = furi_thread_alloc_ex(NULL, 1024, record_consumer_thread, lock);

    furi_thread_start(consumer_thread);
    api_lock_wait_unlock_and_free(lock);

    furi_record_destroy(RECORD_TEST);

    furi_thread_join(consumer_thread);
    mu_check(furi_thread_get_return_code(consumer_thread));
    furi_thread_free(consumer_thread);
}

MU_TEST_SUITE(record_test_suite) {
    MU_RUN_TEST(record_basic_test);
    MU_RUN_TEST(record_ref_count_test);
    MU_RUN_TEST(record_multithread_test);
}

int run_minunit_record_test(void) {
    MU_RUN_SUITE(record_test_suite);
    return MU_EXIT_CODE;
}
