# AGENTS.md

Instructions for coding agents working on the BUSY Bar firmware.

## Important: Custom applications
- There are 2 types of applications: JavaScript apps and Native (or built-in) apps.

| App type | Language | Firmware modification required? | Notes |
| -------- | -------- | ------------------------------- | ----- |
| JavaScript | JavaScript/TypeScript | NO | Intended way for app developers |
| Native | C/C++ | YES | Breaks official updates, requires fork/set of patches |

- Default: build a JavaScript application. Go to `applications_js/AGENTS.md` for further instructions.
- If explicitly asked to build a native app: do it but warn about problems outlined in the table above.

## Read first

- [Code Style](documentation/Code%20Style.dox.md) is **mandatory**. It covers naming, braces, `const`, control
  flow (single return, `do { ... } while(false)`, arrays instead of `switch`), typedefs, object orientation,
  comments and file naming. This file does not repeat it, it only adds to it.
- [Build System](documentation/Build%20System.dox.md) covers `fbt` targets and options.
- [Code review instructions](.github/copilot-instructions.md) list what reviewers check. Make sure your change
  passes them before you call it done.
- [tests/AGENTS.md](tests/AGENTS.md) covers the Python integration tests in `tests/`.

## Project overview

- The firmware runs on two MCUs:
  - STM32U595 ("U5", main firmware, targets `f2x`, built with `TARGET_HW=22`).
  - SiWG917 ("Si917", wireless co-processor, targets `f6x`, built with `TARGET_HW=65`).
  The two talk to each other over the `intercom` service.
- The OS layer is Furi (records, threads, event loops, message queues, pubsub, `FuriString`). It lives in the
  `fbt_layers/core_libs` submodule.
- `applications/` holds `main`, `services`, `settings`, `system` and `debug` apps. Each app is declared in an
  `application.fam` file. `lib/` holds internal and third-party libraries, and `targets/` holds HAL and
  per-target configuration. JavaScript apps live in `applications_js/`.
- Third-party code (submodules, `lib/<vendor>`) must not be modified unless the task explicitly requires it.
  If it does, follow that library's own style.

## Workflow

- Build: `./fbt` (both MCUs), or `./fbt TARGET_HW=22` / `./fbt TARGET_HW=65` for a single one. If the
  change touches shared code, build every target it affects.
- **Always** run `./fbt format` after making changes. `./fbt lint` checks formatting without changing files.
- Unit tests live in `applications/debug/unit_tests` and are built with `./fbt FIRMWARE_APP_SET=unit_tests`.
  When you add or change a library function with non-trivial logic (parsing, validation, state), add or
  extend a test there.
- Keep diffs minimal. Do not reformat, rename or "tidy" code that is unrelated to the task.
- If you change public behaviour or an API that is described in `documentation/`, update the documentation
  in the same change.

## Module structure

A module (library or service) named `foo` normally looks like this:

| File | Contents |
| ---- | -------- |
| `foo.h` | Public API only: opaque type, public enums and structs, callback typedefs, `RECORD_FOO` for services. |
| `foo_i.h` | Private header: the `struct Foo` definition, `TAG`, internal constants, message types, prototypes shared between the module's `.c` files. |
| `foo.c` | Allocation, thread/event-loop setup, entry point. |
| `foo_<concern>.c` | One file per concern, e.g. `foo_api.c`, `foo_connection.c`, `foo_state.c`. |
| `settings/` | Persistent settings built on `setting_provider`, with a versioned schema (`*_interface_v1.[ch]`) and `foo_settings_load/save/reset`. |

- Keep the directory tree shallow, one sub-directory level at most for things like `settings/`, `scenes/`
  or `modules/`. Do not create deep trees, directories with underscore prefixes, or `../../` includes.
- File names carry the module prefix (`foo_state.c`, not `state.c`) and must be unique within the project.
- Put code for each target in **separate source files**, selected per target through `sources` and
  `targets` in `application.fam`. Do not scatter `#if defined(BSB_MCU_SI917)` / `#ifndef ...` through
  function bodies. Code shared by both MCUs goes into a common header and source file.

### Public headers

Every public header starts with a Doxygen file block, followed by `#pragma once`, the includes, and
`extern "C"` guards:

```c
/**
 * @file foo.h
 * @brief One-line summary.
 *
 * Longer explanation: responsibilities, limitations, data formats,
 * data lifetime and threading rules.
 */
#pragma once

#include <core/string.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Parse a foo from a string.
 *
 * @param[in,out] instance pointer to the instance to be filled (can be @c NULL to only validate)
 * @param[in] source pointer to a zero-terminated source string
 * @returns @c true on success, @c false otherwise
 */
bool foo_parse(Foo* instance, const char* source);

#ifdef __cplusplus
}
#endif
```

- Document **every** public type, enum value, struct member (`/**< ... */`) and function.
- Use `@param[in]`, `@param[out]` and `@param[in,out]` for parameters, and `@returns` for return values.
- State the contract explicitly. Who owns the returned memory, and which function frees it? How long does a
  pointer stay valid ("only inside the callback", "until the next load")? Which parameters may be `NULL`?
  What counts as success and what counts as failure? Use `@note` and `@warning` for this.
- Private headers (`*_i.h`) and `.c` files need little or no documentation. Clear names do that job.

## Services

Services follow one architecture. Copy it rather than inventing a new one.

- The service has an entry point `int32_t foo_srv(void* arg)`. It calls a `static Foo* foo_alloc(void)`
  that allocates everything, subscribes to events, and calls `furi_record_create(RECORD_FOO, instance)`
  **last**. The service then runs its `FuriEventLoop` (or its library's poll loop).
- The service thread is the only thread that touches the service state. Do not share state between threads
  behind mutexes.
- Public API functions (in `foo_api.c`) are thin wrappers:
  1. `furi_check()` every argument.
  2. Fill in a typed message on the stack.
  3. Send it to the service thread and block on an `api_lock` until it has been handled.
  4. Return the result, which the handler wrote through a pointer in the message.
- Messages hold a `type` enum and a `union` of **one struct per message type**
  (`FooApiMessageSetConfig`, `FooApiMessagePublish`, ...). Do not use generic `param_int`, `param_bool`
  or `void*` fields that every message shares.
- Dispatch messages through a `static const` handler table indexed by the type enum. Do not use a long
  `switch`:

  ```c
  static const FooApiMessageHandler foo_api_message_handlers[] = {
      [FooApiMessageTypeGetStatus] = foo_get_status_api_message_handler,
      [FooApiMessageTypeSetConfig] = foo_set_config_api_message_handler,
  };

  static_assert(COUNT_OF(foo_api_message_handlers) == FooApiMessageTypeMax);
  ```

- Publish state through `FuriState` / `FuriPubSub`, and subscribe to other services' state the same way.
  Send other services' events to your own thread before acting on them.
- Group a service's internal functions in `foo_i.h` by concern, e.g. `// Network management`.

## GUI applications

- Use `SceneManager` with one file per scene (`foo_scene_<name>.c`), each exporting a `const Scene` with
  `enter`, `exit` and `event` callbacks plus a per-scene data struct. Collect the scenes in a table indexed
  by a `FooSceneId` enum.
- Wrap GUI calls in `with_gui(...)`. Free everything in `on_exit` that was allocated in `on_enter`.
- Send events from widgets to the scene manager as custom events. Do not change app state directly from
  widget callbacks.

## Coding conventions (in addition to Code Style)

### Naming

- **Every** function, including `static` helpers, carries the module prefix: `foo_parse_header()`, not
  `parse_header()`, `load()` or `has_pending_requests()`.
- The *this* parameter is always called `instance`, never a name made from the module or type
  (`foo`, `app`, `ctx`, ...).
- Name functions by role:
  - `foo_alloc` / `foo_free` for heap objects, `foo_init` / `foo_reset` for objects you don't own.
  - `foo_*_callback` for callbacks and `foo_*_handler` for table-dispatched handlers.
  - `foo_*_internal` for the private counterpart of a public function.
- Callback typedefs are `FooSomethingCallback`, and `void* context` is always their last parameter. Inside a
  callback, assign `context` to a typed local (`Foo* instance = context;`) and `furi_assert` it first.
- Put the unit or meaning into the name: `timeout_ms`, `delay_us`, `heap_size_kib`, `data_size`, `str_len`,
  `items_count`, `max_count`. Boolean fields and variables start with `is_`, `has_` or `should_`. Output
  parameters are named `out`/`out_*` or documented as `[out]`.
- Enums are named `FooBarValue`, and their last member is `FooBarMax`, used for bounds checks and table
  sizes.
- Empty parameter lists are written `(void)`.

### Constants and tables

- Do not use magic numbers. Give thresholds, filter coefficients, timeouts, queue sizes, buffer limits and
  JSON keys a named `#define` in parentheses, e.g. `#define FOO_RECONNECT_DELAY_MAX_MS (60000UL)`.
- Build paths with the path macros (`APP_DATA_PATH()`, `SHARED_IMG_PATH()`, `EXT_PATH()`, ...) and
  compose them in macros rather than at runtime.
- Write enum-to-value maps as `static const` arrays with designated initializers
  (`[FooModeOpen] = "open"`), followed by `static_assert(COUNT_OF(map) == FooModeMax);`.
- Keep internal data structures typed: enums for indices, `StringSlice` for non-owning string views, and
  separate structs for distinct concepts.

### Error handling and safety

- Check the arguments of **public** functions with `furi_check()`, which stays active in release builds.
  Use `furi_assert()` for invariants inside `static` functions and callbacks. Use `furi_crash()` for states
  that cannot happen, such as an unknown enum value in a dispatcher.
- `malloc` never returns `NULL`, always returns zeroed memory, and aborts when it fails (see the review
  instructions). So do not check its result, and do not set fields to `0`/`NULL`/`false` right after
  allocating.
- Release resources in the same function that acquired them, in reverse order, on **every** path. This
  applies to `furi_record_open`/`furi_record_close`, `*_alloc`/`*_free` and `storage_file_open`/`close`. The
  single-exit `do { ... } while(false)` pattern exists for this.
- Validate all external data (files, JSON, network, intercom frames, user input) before using it: check
  sizes against explicit maximums, check JSON item types, range-check numbers, verify that the entire input
  was consumed. Log each distinct failure with its own `FURI_LOG_E`/`FURI_LOG_W` message.
- Name a function's result variable after what it means: `success`, `is_success`, `is_valid`, `can_parse`,
  `consumed_len`.
- Do not write `if(x == true)` / `if(x == false)`. Write `if(x)` / `if(!x)`.

### Logging

- Each module defines `#define TAG "FooBar"` (CamelCase) in its private header or `.c` file, and logs with
  `FURI_LOG_E/W/I/D(TAG, ...)`. Do not wrap the logging API in module-specific macros.
- Log messages are short, specific and include the relevant values (`"Failed to read file: expected %llu,
  read %llu bytes"`).

### Functions and comments

- Keep functions short. If a function grows past roughly 40–50 lines, or handles several unrelated concerns
  (reading hardware, filtering, publishing events, starting subsystems), split it into well-named `static`
  helpers.
- Use a `static` helper for a repeated sequence instead of copy-pasting it. Do not assign the same field
  twice in one function.
- Do not leave dead code behind: no `#if 0` blocks, commented-out code, debug-only `#define`s left
  disabled, or bare `// TODO` notes without context.
- Use comments only to explain *why*, e.g. a protocol reference, a non-obvious constraint, or an intentional
  "warn but continue" choice. Do not narrate *what* the code does. Avoid comments inside implementations,
  rely on clear, unambiguous naming instead.

## Before finishing

1. The change is minimal and limited to the task.
2. `./fbt format` has been run, and the affected targets build.
3. Every new public API is fully documented in its header, including ownership and lifetime rules.
4. Every exit path releases what it acquired, and every external input is validated.
5. Tests are added or updated where the logic warrants it.
