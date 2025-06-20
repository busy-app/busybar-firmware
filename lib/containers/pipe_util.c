#include "pipe_util.h"

bool pipe_copy_until(PipeSide* source, PipeSide* dest, const char* terminator) {
    furi_check(source);
    furi_check(terminator);
    size_t terminator_len = strlen(terminator);
    size_t matched_cnt = 0;

    while(1) {
        char c;
        if(pipe_receive(source, &c, sizeof(c)) != sizeof(c)) return false;

        size_t match_flush_cnt = 0;
        if(c == terminator[matched_cnt]) {
            match_flush_cnt = 0;
            matched_cnt++;
        } else if(c == terminator[0]) {
            match_flush_cnt = matched_cnt;
            matched_cnt = 1;
        } else {
            match_flush_cnt = matched_cnt;
            matched_cnt = 0;
        }

        if(matched_cnt == terminator_len) return true;

        if(dest) {
            if(match_flush_cnt) {
                if(pipe_send(dest, terminator, match_flush_cnt) != match_flush_cnt) return false;
            }
            if(!matched_cnt) {
                if(pipe_send(dest, &c, sizeof(c)) != sizeof(c)) return false;
            }
        }
    }
}
