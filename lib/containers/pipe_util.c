#include "pipe_util.h"

static void kmp_build_failure(const char* pattern, size_t len, size_t* failure) {
    failure[0] = 0;
    for(size_t i = 1; i < len; i++) {
        size_t j = failure[i - 1];
        while(j > 0 && pattern[i] != pattern[j]) {
            j = failure[j - 1];
        }
        failure[i] = (pattern[i] == pattern[j]) ? j + 1 : 0;
    }
}

bool pipe_copy_until(PipeSide* source, PipeSide* dest, const char* terminator) {
    furi_check(source);
    furi_check(terminator);
    const size_t terminator_len = strlen(terminator);
    furi_check(terminator_len > 0);

    size_t failure[terminator_len];
    kmp_build_failure(terminator, terminator_len, failure);

    size_t matched_cnt = 0;

    while(1) {
        char c;
        if(pipe_receive(source, &c, sizeof(c)) != sizeof(c)) return false;

        while(matched_cnt > 0 && c != terminator[matched_cnt]) {
            size_t new_matched = failure[matched_cnt - 1];
            if(dest) {
                size_t flush_count = matched_cnt - new_matched;
                if(pipe_send(dest, terminator, flush_count) != flush_count) return false;
            }
            matched_cnt = new_matched;
        }

        if(c == terminator[matched_cnt]) {
            matched_cnt++;
            if(matched_cnt == terminator_len) return true;
        } else {
            if(dest) {
                if(pipe_send(dest, &c, sizeof(c)) != sizeof(c)) return false;
            }
        }
    }
}
