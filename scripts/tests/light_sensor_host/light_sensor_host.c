#include <minunit_vars.h>

extern int run_minunit_light_sensor_test(void);

void minunit_print_progress(void) {
}

void minunit_print_fail(const char* error) {
    fprintf(stderr, "%s\n", error);
}

int main(void) {
    const int result = run_minunit_light_sensor_test();
    MU_REPORT();
    return result;
}
