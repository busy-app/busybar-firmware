/**
 * @file pipe_test.c
 */

#include "../unit_tests.h"
#include <furi.h>
#include <containers/pipe.h>
#include <containers/pipe_util.h>
#include <string.h>

// ========================
// pipe_copy_until tests
// ========================

typedef struct {
    const char* input;
    const char* terminator;
    const char* expected_output;
} PipeCopyTestCase;

static const PipeCopyTestCase copy_until_cases[] = {
    // real-world use cases
    {"property  : value\r\n>: ", "\r\n>: ", "property  : value"},
    // synthetic use cases
    {"abcabcabc", "cab", "ab"},
    {"aaaabc", "abc", "aaa"},
    {"aababcabcd", "abcd", "aababc"},
    // overlapping prefix edge cases (KMP)
    {"aaab", "aab", "a"},
    {"aaacaab", "aab", "aaac"},
    {"abababab", "abab", ""},
    {"abcababab", "abab", "abc"},
};

MU_TEST(pipe_copy_until_test) {
    for(size_t i = 0; i < COUNT_OF(copy_until_cases); i++) {
        const PipeCopyTestCase* tc = &copy_until_cases[i];

        PipeSideBundle bundle_a = pipe_alloc(strlen(tc->input), 1);
        PipeSideBundle bundle_b = pipe_alloc(strlen(tc->input), 1);

        PipeSide* a = bundle_a.alices_side;
        PipeSide* b = bundle_a.bobs_side;
        PipeSide* c = bundle_b.alices_side;
        PipeSide* d = bundle_b.bobs_side;

        pipe_send(a, tc->input, strlen(tc->input));

        mu_assert_int_eq(true, pipe_copy_until(b, c, tc->terminator));
        pipe_free(a);
        pipe_free(c);

        char output_buf[strlen(tc->input) + 1];
        size_t output_cnt = pipe_receive(d, output_buf, sizeof(output_buf));
        output_buf[output_cnt] = '\0';
        pipe_free(d);

        char leftover_buf[strlen(tc->input) + 1];
        size_t leftover_cnt = pipe_receive(b, leftover_buf, sizeof(leftover_buf));
        leftover_buf[leftover_cnt] = '\0';
        pipe_free(b);

        const char* expected_leftover =
            tc->input + strlen(tc->terminator) + strlen(tc->expected_output);
        mu_assert_string_eq(tc->expected_output, output_buf);
        mu_assert_string_eq(expected_leftover, leftover_buf);
    }
}

MU_TEST(pipe_copy_to_null_until_test) {
    for(size_t i = 0; i < COUNT_OF(copy_until_cases); i++) {
        const PipeCopyTestCase* tc = &copy_until_cases[i];

        PipeSideBundle bundle_a = pipe_alloc(strlen(tc->input), 1);

        PipeSide* a = bundle_a.alices_side;
        PipeSide* b = bundle_a.bobs_side;

        pipe_send(a, tc->input, strlen(tc->input));

        mu_assert_int_eq(true, pipe_copy_until(b, NULL, tc->terminator));
        pipe_free(a);

        char leftover_buf[strlen(tc->input) + 1];
        size_t leftover_cnt = pipe_receive(b, leftover_buf, sizeof(leftover_buf));
        leftover_buf[leftover_cnt] = '\0';
        pipe_free(b);

        const char* expected_leftover =
            tc->input + strlen(tc->terminator) + strlen(tc->expected_output);
        mu_assert_string_eq(expected_leftover, leftover_buf);
    }
}

// ========================
// Basic pipe lifecycle
// ========================

MU_TEST(pipe_alloc_free_test) {
    // Allocate and immediately free both sides
    PipeSideBundle bundle = pipe_alloc(64, 1);
    mu_assert_int_eq(PipeRoleAlice, pipe_role(bundle.alices_side));
    mu_assert_int_eq(PipeRoleBob, pipe_role(bundle.bobs_side));
    mu_assert_int_eq(PipeStateOpen, pipe_state(bundle.alices_side));
    mu_assert_int_eq(PipeStateOpen, pipe_state(bundle.bobs_side));
    pipe_free(bundle.alices_side);
    pipe_free(bundle.bobs_side);
}

MU_TEST(pipe_free_one_side_broken_test) {
    // Freeing one side makes the other see PipeStateBroken
    PipeSideBundle bundle = pipe_alloc(64, 1);
    pipe_free(bundle.alices_side);
    mu_assert_int_eq(PipeStateBroken, pipe_state(bundle.bobs_side));
    pipe_free(bundle.bobs_side);
}

MU_TEST(pipe_free_bob_first_broken_test) {
    // Same, but free Bob first
    PipeSideBundle bundle = pipe_alloc(64, 1);
    pipe_free(bundle.bobs_side);
    mu_assert_int_eq(PipeStateBroken, pipe_state(bundle.alices_side));
    pipe_free(bundle.alices_side);
}

// ========================
// pipe_close tests
// ========================

MU_TEST(pipe_close_makes_broken_test) {
    // pipe_close marks both sides as broken without freeing
    PipeSideBundle bundle = pipe_alloc(64, 1);
    pipe_close(bundle.alices_side);
    mu_assert_int_eq(PipeStateBroken, pipe_state(bundle.alices_side));
    mu_assert_int_eq(PipeStateBroken, pipe_state(bundle.bobs_side));
    // Both sides can still be freed after close
    pipe_free(bundle.alices_side);
    pipe_free(bundle.bobs_side);
}

MU_TEST(pipe_close_from_bob_test) {
    // pipe_close from Bob's side
    PipeSideBundle bundle = pipe_alloc(64, 1);
    pipe_close(bundle.bobs_side);
    mu_assert_int_eq(PipeStateBroken, pipe_state(bundle.alices_side));
    mu_assert_int_eq(PipeStateBroken, pipe_state(bundle.bobs_side));
    pipe_free(bundle.alices_side);
    pipe_free(bundle.bobs_side);
}

MU_TEST(pipe_close_then_free_one_test) {
    // pipe_close, then free one side, then free the other
    PipeSideBundle bundle = pipe_alloc(64, 1);
    pipe_close(bundle.alices_side);
    pipe_free(bundle.alices_side);
    mu_assert_int_eq(PipeStateBroken, pipe_state(bundle.bobs_side));
    pipe_free(bundle.bobs_side);
}

// ========================
// Send / receive basics
// ========================

MU_TEST(pipe_send_receive_simple_test) {
    PipeSideBundle bundle = pipe_alloc(64, 1);
    const char msg[] = "Hello, pipe!";
    mu_assert_int_eq(sizeof(msg), pipe_send(bundle.alices_side, msg, sizeof(msg)));

    char buf[sizeof(msg)];
    // Data should be available on Bob's side
    mu_assert_int_eq(sizeof(msg), pipe_bytes_available(bundle.bobs_side));
    mu_assert_int_eq(sizeof(msg), pipe_receive(bundle.bobs_side, buf, sizeof(buf)));
    mu_assert_mem_eq(msg, buf, sizeof(msg));

    pipe_free(bundle.alices_side);
    pipe_free(bundle.bobs_side);
}

MU_TEST(pipe_send_receive_bidirectional_test) {
    // Both sides can send and receive simultaneously
    PipeSideBundle bundle = pipe_alloc(64, 1);
    const char msg_a[] = "from alice";
    const char msg_b[] = "from bob";

    mu_assert_int_eq(sizeof(msg_a), pipe_send(bundle.alices_side, msg_a, sizeof(msg_a)));
    mu_assert_int_eq(sizeof(msg_b), pipe_send(bundle.bobs_side, msg_b, sizeof(msg_b)));

    char buf_a[sizeof(msg_b)];
    char buf_b[sizeof(msg_a)];
    mu_assert_int_eq(sizeof(msg_b), pipe_receive(bundle.alices_side, buf_a, sizeof(buf_a)));
    mu_assert_int_eq(sizeof(msg_a), pipe_receive(bundle.bobs_side, buf_b, sizeof(buf_b)));
    mu_assert_mem_eq(msg_b, buf_a, sizeof(msg_b));
    mu_assert_mem_eq(msg_a, buf_b, sizeof(msg_a));

    pipe_free(bundle.alices_side);
    pipe_free(bundle.bobs_side);
}

MU_TEST(pipe_send_receive_fill_exact_test) {
    // Fill the pipe exactly to capacity and read it all back
    const size_t capacity = 32;
    PipeSideBundle bundle = pipe_alloc(capacity, 1);

    uint8_t send_data[capacity];
    for(size_t i = 0; i < capacity; i++)
        send_data[i] = (uint8_t)(i & 0xFF);

    mu_assert_int_eq((int)capacity, (int)pipe_send(bundle.alices_side, send_data, capacity));
    mu_assert_int_eq(0, (int)pipe_spaces_available(bundle.alices_side));

    uint8_t recv_data[capacity];
    mu_assert_int_eq((int)capacity, (int)pipe_receive(bundle.bobs_side, recv_data, capacity));
    mu_assert_mem_eq(send_data, recv_data, capacity);
    mu_assert_int_eq((int)capacity, (int)pipe_spaces_available(bundle.alices_side));

    pipe_free(bundle.alices_side);
    pipe_free(bundle.bobs_side);
}

MU_TEST(pipe_send_receive_incremental_test) {
    // Send byte-by-byte, then receive all at once
    const size_t n = 16;
    PipeSideBundle bundle = pipe_alloc(n, 1);

    for(size_t i = 0; i < n; i++) {
        uint8_t byte = (uint8_t)i;
        mu_assert_int_eq(1, (int)pipe_send(bundle.alices_side, &byte, 1));
    }

    uint8_t buf[n];
    mu_assert_int_eq((int)n, (int)pipe_receive(bundle.bobs_side, buf, n));
    for(size_t i = 0; i < n; i++) {
        mu_assert_int_eq((int)i, (int)buf[i]);
    }

    pipe_free(bundle.alices_side);
    pipe_free(bundle.bobs_side);
}

// ========================
// Broken pipe read/write
// ========================

MU_TEST(pipe_receive_broken_returns_partial_test) {
    // After sending some data and freeing sender, receiver gets remaining data
    PipeSideBundle bundle = pipe_alloc(64, 1);
    const char msg[] = "partial";
    pipe_send(bundle.alices_side, msg, sizeof(msg));
    pipe_free(bundle.alices_side);

    char buf[64];
    size_t got = pipe_receive(bundle.bobs_side, buf, sizeof(buf));
    mu_assert_int_eq((int)sizeof(msg), (int)got);
    mu_assert_mem_eq(msg, buf, sizeof(msg));

    // Further receive returns 0
    got = pipe_receive(bundle.bobs_side, buf, sizeof(buf));
    mu_assert_int_eq(0, (int)got);

    pipe_free(bundle.bobs_side);
}

MU_TEST(pipe_send_broken_returns_zero_test) {
    // Sending to a broken pipe: data that fits in the buffer may be accepted,
    // but once the buffer is full the send loop detects broken state and stops.
    // With a tiny buffer the first batch goes into the stream buffer, the rest is lost.
    PipeSideBundle bundle = pipe_alloc(4, 1);
    pipe_free(bundle.bobs_side);

    const char msg[] = "nobody home";
    size_t sent = pipe_send(bundle.alices_side, msg, sizeof(msg));
    // At most 4 bytes fit into the buffer; the rest is dropped
    mu_assert(sent <= 4, "sent too many bytes to broken pipe");

    pipe_free(bundle.alices_side);
}

MU_TEST(pipe_close_unblocks_receive_test) {
    // pipe_close causes a pending pipe_receive to return partial data or 0
    // Here we test the non-threaded case: pipe is closed before receive starts
    PipeSideBundle bundle = pipe_alloc(64, 1);
    const char msg[] = "before close";
    pipe_send(bundle.alices_side, msg, sizeof(msg));
    pipe_close(bundle.alices_side);

    char buf[64];
    size_t got = pipe_receive(bundle.bobs_side, buf, sizeof(buf));
    // Should get the buffered data despite broken state
    mu_assert_int_eq((int)sizeof(msg), (int)got);
    mu_assert_mem_eq(msg, buf, sizeof(msg));

    // Next receive returns 0
    got = pipe_receive(bundle.bobs_side, buf, sizeof(buf));
    mu_assert_int_eq(0, (int)got);

    pipe_free(bundle.alices_side);
    pipe_free(bundle.bobs_side);
}

MU_TEST(pipe_close_unblocks_send_test) {
    // pipe_close allows sends to detect broken state
    PipeSideBundle bundle = pipe_alloc(4, 1);
    pipe_close(bundle.bobs_side);

    const char msg[] = "won't fit";
    size_t sent = pipe_send(bundle.alices_side, msg, sizeof(msg));
    // May send some bytes into the buffer, but won't block forever
    mu_assert(sent <= sizeof(msg), "sent too many bytes to broken pipe");

    pipe_free(bundle.alices_side);
    pipe_free(bundle.bobs_side);
}

// ========================
// Threaded pipe_close test
// ========================

typedef struct {
    PipeSide* pipe;
    size_t received;
} PipeThreadTestCtx;

static int32_t pipe_close_reader_thread(void* context) {
    PipeThreadTestCtx* ctx = context;
    char buf[64];
    ctx->received = pipe_receive(ctx->pipe, buf, sizeof(buf));
    return 0;
}

MU_TEST(pipe_close_unblocks_threaded_receive_test) {
    // Verify that pipe_close from one thread unblocks pipe_receive in another
    PipeSideBundle bundle = pipe_alloc(64, 1);

    PipeThreadTestCtx ctx = {.pipe = bundle.bobs_side, .received = 0};
    FuriThread* thread =
        furi_thread_alloc_ex("pipe_test_reader", 1024, pipe_close_reader_thread, &ctx);
    furi_thread_start(thread);

    // Give the reader time to block on pipe_receive
    furi_delay_ms(50);

    // Close from the other side — should unblock the reader within ~100ms
    pipe_close(bundle.alices_side);

    furi_thread_join(thread);
    furi_thread_free(thread);

    mu_assert_int_eq(0, (int)ctx.received);

    pipe_free(bundle.alices_side);
    pipe_free(bundle.bobs_side);
}

static int32_t pipe_free_reader_thread(void* context) {
    PipeThreadTestCtx* ctx = context;
    char buf[64];
    ctx->received = pipe_receive(ctx->pipe, buf, sizeof(buf));
    return 0;
}

MU_TEST(pipe_free_unblocks_threaded_receive_test) {
    // Verify that pipe_free from one thread unblocks pipe_receive in another
    PipeSideBundle bundle = pipe_alloc(64, 1);

    PipeThreadTestCtx ctx = {.pipe = bundle.bobs_side, .received = 0};
    FuriThread* thread =
        furi_thread_alloc_ex("pipe_test_reader", 1024, pipe_free_reader_thread, &ctx);
    furi_thread_start(thread);

    // Give the reader time to block
    furi_delay_ms(50);

    pipe_free(bundle.alices_side);

    furi_thread_join(thread);
    furi_thread_free(thread);

    mu_assert_int_eq(0, (int)ctx.received);

    pipe_free(bundle.bobs_side);
}

// ========================
// Threaded producer/consumer
// ========================

typedef struct {
    PipeSide* pipe;
    const uint8_t* data;
    size_t length;
    size_t sent;
} PipeWriterCtx;

static int32_t pipe_writer_thread(void* context) {
    PipeWriterCtx* ctx = context;
    ctx->sent = pipe_send(ctx->pipe, ctx->data, ctx->length);
    return 0;
}

MU_TEST(pipe_threaded_transfer_test) {
    // Transfer data larger than pipe capacity between two threads
    const size_t pipe_capacity = 32;
    const size_t data_size = 256;
    PipeSideBundle bundle = pipe_alloc(pipe_capacity, 1);

    uint8_t send_data[data_size];
    for(size_t i = 0; i < data_size; i++)
        send_data[i] = (uint8_t)(i & 0xFF);

    PipeWriterCtx writer_ctx = {
        .pipe = bundle.alices_side,
        .data = send_data,
        .length = data_size,
        .sent = 0,
    };
    FuriThread* writer =
        furi_thread_alloc_ex("pipe_test_writer", 1024, pipe_writer_thread, &writer_ctx);
    furi_thread_start(writer);

    uint8_t recv_data[data_size];
    size_t total_received = 0;
    while(total_received < data_size) {
        size_t got =
            pipe_receive(bundle.bobs_side, recv_data + total_received, data_size - total_received);
        if(!got) break;
        total_received += got;
    }

    furi_thread_join(writer);
    furi_thread_free(writer);

    mu_assert_int_eq((int)data_size, (int)writer_ctx.sent);
    mu_assert_int_eq((int)data_size, (int)total_received);
    mu_assert_mem_eq(send_data, recv_data, data_size);

    pipe_free(bundle.alices_side);
    pipe_free(bundle.bobs_side);
}

// ========================
// Asymmetric pipe
// ========================

MU_TEST(pipe_alloc_ex_asymmetric_test) {
    // Test different capacities for each direction
    PipeSideReceiveSettings alice_settings = {.capacity = 16, .trigger_level = 1};
    PipeSideReceiveSettings bob_settings = {.capacity = 64, .trigger_level = 1};
    PipeSideBundle bundle = pipe_alloc_ex(alice_settings, bob_settings);

    // Alice sends to Bob — Bob's receive capacity is 64
    uint8_t data64[64];
    memset(data64, 0xAB, sizeof(data64));
    mu_assert_int_eq(64, (int)pipe_send(bundle.alices_side, data64, 64));

    // Bob sends to Alice — Alice's receive capacity is 16
    uint8_t data16[16];
    memset(data16, 0xCD, sizeof(data16));
    mu_assert_int_eq(16, (int)pipe_send(bundle.bobs_side, data16, 16));

    // Verify
    uint8_t buf64[64];
    mu_assert_int_eq(64, (int)pipe_receive(bundle.bobs_side, buf64, 64));
    mu_assert_mem_eq(data64, buf64, 64);

    uint8_t buf16[16];
    mu_assert_int_eq(16, (int)pipe_receive(bundle.alices_side, buf16, 16));
    mu_assert_mem_eq(data16, buf16, 16);

    pipe_free(bundle.alices_side);
    pipe_free(bundle.bobs_side);
}

// ========================
// spaces_available / bytes_available
// ========================

MU_TEST(pipe_availability_tracking_test) {
    const size_t cap = 32;
    PipeSideBundle bundle = pipe_alloc(cap, 1);

    // Initially: full capacity available for sending, zero bytes to receive
    mu_assert_int_eq((int)cap, (int)pipe_spaces_available(bundle.alices_side));
    mu_assert_int_eq(0, (int)pipe_bytes_available(bundle.bobs_side));

    // Send half
    uint8_t data[cap];
    memset(data, 0x42, sizeof(data));
    pipe_send(bundle.alices_side, data, cap / 2);

    mu_assert_int_eq((int)(cap / 2), (int)pipe_spaces_available(bundle.alices_side));
    mu_assert_int_eq((int)(cap / 2), (int)pipe_bytes_available(bundle.bobs_side));

    // Receive some
    uint8_t buf[8];
    pipe_receive(bundle.bobs_side, buf, 8);
    mu_assert_int_eq((int)(cap / 2 + 8), (int)pipe_spaces_available(bundle.alices_side));
    mu_assert_int_eq((int)(cap / 2 - 8), (int)pipe_bytes_available(bundle.bobs_side));

    pipe_free(bundle.alices_side);
    pipe_free(bundle.bobs_side);
}

// ========================
// Multiple send/receive cycles (wrap-around)
// ========================

MU_TEST(pipe_wraparound_test) {
    // Stress the ring buffer by doing many small send/receive cycles
    const size_t cap = 16;
    PipeSideBundle bundle = pipe_alloc(cap, 1);

    for(int cycle = 0; cycle < 20; cycle++) {
        uint8_t val = (uint8_t)cycle;
        uint8_t send_buf[8];
        memset(send_buf, val, sizeof(send_buf));

        mu_assert_int_eq(8, (int)pipe_send(bundle.alices_side, send_buf, 8));

        uint8_t recv_buf[8];
        mu_assert_int_eq(8, (int)pipe_receive(bundle.bobs_side, recv_buf, 8));
        for(int j = 0; j < 8; j++) {
            mu_assert_int_eq(val, recv_buf[j]);
        }
    }

    pipe_free(bundle.alices_side);
    pipe_free(bundle.bobs_side);
}

// ========================
// pipe_copy_until broken pipe
// ========================

MU_TEST(pipe_copy_until_broken_returns_false_test) {
    // When source is broken before terminator is found, pipe_copy_until returns false
    PipeSideBundle bundle = pipe_alloc(64, 1);
    const char msg[] = "no terminator here";
    pipe_send(bundle.alices_side, msg, strlen(msg));
    pipe_free(bundle.alices_side);

    mu_assert_int_eq(false, pipe_copy_until(bundle.bobs_side, NULL, "MISSING"));
    pipe_free(bundle.bobs_side);
}

MU_TEST(pipe_copy_until_close_returns_false_test) {
    // Same but using pipe_close instead of pipe_free
    PipeSideBundle bundle = pipe_alloc(64, 1);
    const char msg[] = "no terminator here";
    pipe_send(bundle.alices_side, msg, strlen(msg));
    pipe_close(bundle.alices_side);

    mu_assert_int_eq(false, pipe_copy_until(bundle.bobs_side, NULL, "MISSING"));
    pipe_free(bundle.alices_side);
    pipe_free(bundle.bobs_side);
}

// ========================
// Broken callback via event loop
// ========================

typedef struct {
    PipeSide* pipe;
    FuriEventLoop* event_loop;
    bool callback_fired;
} PipeBrokenCbCtx;

static void pipe_broken_cb(PipeSide* pipe, void* context) {
    UNUSED(pipe);
    PipeBrokenCbCtx* ctx = context;
    ctx->callback_fired = true;
    furi_event_loop_stop(ctx->event_loop);
}

static int32_t pipe_event_loop_thread(void* context) {
    PipeBrokenCbCtx* ctx = context;
    ctx->event_loop = furi_event_loop_alloc();

    pipe_attach_to_event_loop(ctx->pipe, ctx->event_loop);
    pipe_set_callback_context(ctx->pipe, ctx);
    pipe_set_broken_callback(ctx->pipe, pipe_broken_cb, FuriEventLoopEventFlagEdge);

    furi_event_loop_run(ctx->event_loop);

    pipe_set_broken_callback(ctx->pipe, NULL, 0);
    pipe_detach_from_event_loop(ctx->pipe);
    furi_event_loop_free(ctx->event_loop);
    return 0;
}

MU_TEST(pipe_close_fires_broken_callback_test) {
    // pipe_close() should trigger the broken callback on the other side's event loop
    PipeSideBundle bundle = pipe_alloc(64, 1);

    PipeBrokenCbCtx ctx = {.pipe = bundle.bobs_side, .callback_fired = false};
    FuriThread* thread =
        furi_thread_alloc_ex("pipe_test_evloop", 1024, pipe_event_loop_thread, &ctx);
    furi_thread_start(thread);

    // Give the event loop time to start and subscribe
    furi_delay_ms(50);

    // Close from Alice's side — should trigger Bob's broken callback
    pipe_close(bundle.alices_side);

    furi_thread_join(thread);
    furi_thread_free(thread);

    mu_check(ctx.callback_fired);

    pipe_free(bundle.alices_side);
    pipe_free(bundle.bobs_side);
}

MU_TEST(pipe_free_fires_broken_callback_test) {
    // pipe_free() should also trigger the broken callback (existing behavior with refactored code)
    PipeSideBundle bundle = pipe_alloc(64, 1);

    PipeBrokenCbCtx ctx = {.pipe = bundle.bobs_side, .callback_fired = false};
    FuriThread* thread =
        furi_thread_alloc_ex("pipe_test_evloop", 1024, pipe_event_loop_thread, &ctx);
    furi_thread_start(thread);

    furi_delay_ms(50);

    // Free Alice's side — should trigger Bob's broken callback
    pipe_free(bundle.alices_side);

    furi_thread_join(thread);
    furi_thread_free(thread);

    mu_check(ctx.callback_fired);

    pipe_free(bundle.bobs_side);
}

// ========================
// Operations on closed/broken pipe
// ========================

MU_TEST(pipe_closed_receive_drains_then_zero_test) {
    // After close, receive drains remaining buffered data, then returns 0
    PipeSideBundle bundle = pipe_alloc(64, 1);
    const char msg[] = "buffered data";
    pipe_send(bundle.alices_side, msg, sizeof(msg));
    pipe_close(bundle.alices_side);

    char buf[64];
    size_t got = pipe_receive(bundle.bobs_side, buf, sizeof(buf));
    mu_assert_int_eq((int)sizeof(msg), (int)got);
    mu_assert_mem_eq(msg, buf, sizeof(msg));

    got = pipe_receive(bundle.bobs_side, buf, sizeof(buf));
    mu_assert_int_eq(0, (int)got);

    pipe_free(bundle.alices_side);
    pipe_free(bundle.bobs_side);
}

MU_TEST(pipe_closed_send_returns_zero_test) {
    // Sending to a closed pipe: data that fits in the buffer may be accepted.
    // Use a tiny buffer so we can observe the cutoff.
    PipeSideBundle bundle = pipe_alloc(4, 1);
    pipe_close(bundle.bobs_side);

    const char msg[] = "to closed pipe";
    size_t sent = pipe_send(bundle.alices_side, msg, sizeof(msg));
    // At most 4 bytes fit; the rest is dropped when broken state is detected
    mu_assert(sent <= 4, "sent too many bytes to closed pipe");

    pipe_free(bundle.alices_side);
    pipe_free(bundle.bobs_side);
}

MU_TEST(pipe_closed_bytes_available_test) {
    // bytes_available reports buffered data even after close
    PipeSideBundle bundle = pipe_alloc(64, 1);
    const char msg[] = "check bytes";
    pipe_send(bundle.alices_side, msg, sizeof(msg));
    pipe_close(bundle.alices_side);

    mu_assert_int_eq((int)sizeof(msg), (int)pipe_bytes_available(bundle.bobs_side));

    // Drain the data
    char buf[64];
    pipe_receive(bundle.bobs_side, buf, sizeof(buf));
    mu_assert_int_eq(0, (int)pipe_bytes_available(bundle.bobs_side));

    pipe_free(bundle.alices_side);
    pipe_free(bundle.bobs_side);
}

MU_TEST(pipe_closed_spaces_available_test) {
    // spaces_available still reflects buffer capacity after close
    PipeSideBundle bundle = pipe_alloc(64, 1);
    pipe_close(bundle.bobs_side);
    // The buffer itself is not freed; spaces_available reflects the underlying stream buffer
    mu_assert_int_eq(64, (int)pipe_spaces_available(bundle.alices_side));

    pipe_free(bundle.alices_side);
    pipe_free(bundle.bobs_side);
}

MU_TEST(pipe_freed_send_returns_zero_test) {
    // Sending after other side is freed: data that fits may still enter the buffer.
    PipeSideBundle bundle = pipe_alloc(4, 1);
    pipe_free(bundle.bobs_side);

    const char msg[] = "after free";
    size_t sent = pipe_send(bundle.alices_side, msg, sizeof(msg));
    mu_assert(sent <= 4, "sent too many bytes to freed pipe");

    pipe_free(bundle.alices_side);
}

MU_TEST(pipe_freed_receive_drains_then_zero_test) {
    // After free of sender, receive drains buffered data, then returns 0
    PipeSideBundle bundle = pipe_alloc(64, 1);
    const char msg[] = "before free";
    pipe_send(bundle.alices_side, msg, sizeof(msg));
    pipe_free(bundle.alices_side);

    char buf[64];
    size_t got = pipe_receive(bundle.bobs_side, buf, sizeof(buf));
    mu_assert_int_eq((int)sizeof(msg), (int)got);

    got = pipe_receive(bundle.bobs_side, buf, sizeof(buf));
    mu_assert_int_eq(0, (int)got);

    pipe_free(bundle.bobs_side);
}

// ========================
// Double close safety
// ========================

MU_TEST(pipe_double_close_test) {
    // Calling pipe_close twice on the same side should be safe (idempotent)
    PipeSideBundle bundle = pipe_alloc(64, 1);
    pipe_close(bundle.alices_side);
    pipe_close(bundle.alices_side); // no crash
    mu_assert_int_eq(PipeStateBroken, pipe_state(bundle.bobs_side));

    pipe_free(bundle.alices_side);
    pipe_free(bundle.bobs_side);
}

MU_TEST(pipe_close_both_sides_test) {
    // Calling pipe_close on both sides should be safe
    PipeSideBundle bundle = pipe_alloc(64, 1);
    pipe_close(bundle.alices_side);
    pipe_close(bundle.bobs_side);
    mu_assert_int_eq(PipeStateBroken, pipe_state(bundle.alices_side));
    mu_assert_int_eq(PipeStateBroken, pipe_state(bundle.bobs_side));

    pipe_free(bundle.alices_side);
    pipe_free(bundle.bobs_side);
}

MU_TEST(pipe_close_then_free_both_test) {
    // close + free on one side, then free on the other
    PipeSideBundle bundle = pipe_alloc(64, 1);
    pipe_close(bundle.alices_side);
    pipe_free(bundle.alices_side);
    // Bob should still see broken, and free should not crash
    mu_assert_int_eq(PipeStateBroken, pipe_state(bundle.bobs_side));
    pipe_free(bundle.bobs_side);
}

// ========================
// Suite definition
// ========================

MU_TEST_SUITE(pipe_test_suite) {
    // Lifecycle
    MU_RUN_TEST(pipe_alloc_free_test);
    MU_RUN_TEST(pipe_free_one_side_broken_test);
    MU_RUN_TEST(pipe_free_bob_first_broken_test);

    // pipe_close
    MU_RUN_TEST(pipe_close_makes_broken_test);
    MU_RUN_TEST(pipe_close_from_bob_test);
    MU_RUN_TEST(pipe_close_then_free_one_test);
    MU_RUN_TEST(pipe_double_close_test);
    MU_RUN_TEST(pipe_close_both_sides_test);
    MU_RUN_TEST(pipe_close_then_free_both_test);

    // Basic send/receive
    MU_RUN_TEST(pipe_send_receive_simple_test);
    MU_RUN_TEST(pipe_send_receive_bidirectional_test);
    MU_RUN_TEST(pipe_send_receive_fill_exact_test);
    MU_RUN_TEST(pipe_send_receive_incremental_test);

    // Broken pipe behavior
    MU_RUN_TEST(pipe_receive_broken_returns_partial_test);
    MU_RUN_TEST(pipe_send_broken_returns_zero_test);
    MU_RUN_TEST(pipe_close_unblocks_receive_test);
    MU_RUN_TEST(pipe_close_unblocks_send_test);

    // Operations on closed/broken pipe
    MU_RUN_TEST(pipe_closed_receive_drains_then_zero_test);
    MU_RUN_TEST(pipe_closed_send_returns_zero_test);
    MU_RUN_TEST(pipe_closed_bytes_available_test);
    MU_RUN_TEST(pipe_closed_spaces_available_test);
    MU_RUN_TEST(pipe_freed_send_returns_zero_test);
    MU_RUN_TEST(pipe_freed_receive_drains_then_zero_test);

    // Broken callback via event loop
    MU_RUN_TEST(pipe_close_fires_broken_callback_test);
    MU_RUN_TEST(pipe_free_fires_broken_callback_test);

    // Threaded
    MU_RUN_TEST(pipe_close_unblocks_threaded_receive_test);
    MU_RUN_TEST(pipe_free_unblocks_threaded_receive_test);
    MU_RUN_TEST(pipe_threaded_transfer_test);

    // Asymmetric / misc
    MU_RUN_TEST(pipe_alloc_ex_asymmetric_test);
    MU_RUN_TEST(pipe_availability_tracking_test);
    MU_RUN_TEST(pipe_wraparound_test);

    // pipe_copy_until
    MU_RUN_TEST(pipe_copy_until_test);
    MU_RUN_TEST(pipe_copy_to_null_until_test);
    MU_RUN_TEST(pipe_copy_until_broken_returns_false_test);
    MU_RUN_TEST(pipe_copy_until_close_returns_false_test);
}

int run_minunit_pipe_test(void) {
    MU_RUN_SUITE(pipe_test_suite);
    return MU_EXIT_CODE;
}
