---
applyTo: "applications/**/*.c,applications/**/*.h,applications/**/*.cpp,applications/**/*.hpp,targets/**/*.c,targets/**/*.h"
---

# C/C++ firmware review checks

Base your review on `documentation/Code Style.dox.md` and `documentation/Services.dox.md`; `AGENTS.md` adds
project-specific rules. These are the checks that matter most often.

## Naming and structure

- Every function, including `static` helpers, carries the module prefix (`foo_parse_header`, not `parse_header`).
- The *this* parameter is always named `instance`.
- Callback typedefs are `FooSomethingCallback` and take `void* context` as their last parameter.
- Message dispatch uses a `static const` handler table plus `static_assert(COUNT_OF(table) == ...Max)`, not a long `switch`.
- Enums end with `FooBarMax`; empty parameter lists are written `(void)`.
- No magic numbers: thresholds, timeouts, buffer sizes and JSON keys are named `#define`s in parentheses.

## Resources and memory

- The allocator never returns `NULL`, returns zeroed memory and aborts on failure. Flag code that checks `malloc` for `NULL` or re-zeroes fields right after allocating.
- Every acquired resource (`furi_record_open`, `*_alloc`, `storage_file_open`, mutexes, record handles) must be released on every exit path. The single-exit `do { ... } while(false)` pattern exists for exactly this.
- Event-loop subscriptions, pubsub subscriptions and threads need a matching teardown, including on error paths.

## External data

- Data from files, JSON, network, intercom frames or user input must be size-checked against an explicit maximum, type-checked, range-checked and fully consumed before use.
- Each distinct failure gets its own `FURI_LOG_E`/`FURI_LOG_W` message that includes the relevant values.

## Concurrency and targets

- Service state is owned by the service thread. Flag shared state behind ad-hoc mutexes, and pubsub callbacks that touch service state directly instead of forwarding it to the service thread.
- Per-target code belongs in separate source files selected via `application.fam`. Flag `#if defined(BSB_MCU_SI917)` or similar blocks inside function bodies.

## Review runtime

There is no compiler, linker, `./fbt` build or unit-test runner available, so do not claim that a chang builds, links or passes tests.
