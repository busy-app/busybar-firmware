# Code Style {#code-style}

This is a stub page about the code style.

## Introduction

### Automatic formatting

After making any changes, the code must be formatted by running `./fbt format` from the project root directory. In case it is preferable to use the built-in formatting feature in the IDE/editor, it must use `clang-format` with the configuration file provided in the aforementioned directory.

### Clarity

Always try to make the code as clear and straightforward as possible. Avoid "clever code", obscure language constructions or other kinds of obfuscation.

### Comments

Avoid comments unless it will greatly improve clarity. Most of the time a comment can be avoided by extracting a function or by a more descriptive variable name.

If using an LLM, care must be taken to rephrase or remove the generated comments, as they tend to be overly verbose.

### Functions

Functions must be relatively short and do *roughly* one thing. They must have descriptive names that make it clear what the function is doing.

## File structure

- File names should be as short as possible, but remain descriptive and unique for easy searching.
- When modifying or extending an existing third-party library, adopt its code style when practical.
- Source and header files must be in *snake case*, e.g. `my_awesome_header.h, ingenious_source.c`
- File names should not be overly generic, e.g. `my_app_settings.c` instead of `settings.c`

## Programming conventions

#### Preferred case style

Function and variable names must be in *snake case*:

```C
void do_something(void) {
    // .....
    char my_awesome_variable = 'a';
    // .....
}
```

Custom types must be in *camel case*:

```C
typedef struct {
    // ...
} MyAwesomeStruct;

typedef void (*VeryImportantCallback)(void* context);
```

#### Curly braces

Always put curly braces around conditional blocks, even when the branch body is a single line:

```C
// Not ideal
if(my_condition)
    x = 42;
    y = 69; // Will always execute. Not nice!

// Also not ideal
if(my_condition) x = 42;

// Perfect, the branch has clear boundaries!
if(my_condition) {
    x = 42;
}
```

#### Const

If a variable does not have to change, mark it as `const`. This applies both to global and local variables.

```C
float prism_volume(float w, float l, float h) {
    // Storing intermediate calculation in a local const variable
    const float base_area = w * l;
    return base_area * h;
}

```

If a function parameter not an input parameter, mark it as `const`, unless it is of a primitive type.

```C
uint32_t my_container_get_count(const MyContainer* instance) {
    // No need to mutate instance, just reading a member value
    return instance->count;
}
```

#### Control flow

Where possible, replace a `switch` with array indexing:

```C
// Not ideal
switch(my_index) {
case 0:
    x = 12;
    break;
case 1:
    x = 45;
    break;
case 3:
    x = 59;
    break;
case 5:
    x = 67;
    break;
default:
    x = DEFAULT_VALUE;
}

// Better
x = DEFAULT_VALUE;

static const int my_choices[] = {12, 45, 59, 42, 67};
if(my_index >= 0 && my_index < COUNT_OF(my_choices)) {
    x = my_choices[my_enum];
}
```

If the above is not practical, e.g. `my_index` values are not contigious, it is preferable to use `if...else` over `switch`:

```C
// Not ideal
switch(my_value) {
case 12:
    x = 12;
    break;
case 36:
    x = 45;
    break;
case 74:
    x = 59;
    break;
default:
    x = DEFAULT_VALUE;
}

// Better
if(my_value == 12) {
    x = 12;
} else if(my_value == 45) {
    x = 45;
} else if(my_value == 74) {
    x = 59;
} else {
    x = DEFAULT_VALUE;
}

```

It is preferable to always use one return statement per function. Among other benefits, it reduces the chance of forgetting to deallocate heap memory due to an early return.

```C
// Not ideal
bool do_it_now(void) {
    Resource* res = resource_alloc(100500);

    if(!step_one(res)) {
        resource_free(res);
        return false;
    }

    if(!step_two(res)) {
        // Whoops! Forgot to free `res`
        return false;
    }

    resource_free(res);

    return true;
}

// Better
bool do_it_now(void) {
    bool success = true;

    Resource* res = resource_alloc(100500);

    do {
        if(!step_one(res)) {
            break;
        }

        if(!step_two(res)) {
            break;
        }

        success = true;
    } while(false);

    // Now it's much harder to forget to free `res`
    resource_free(res);

    return success;
}

```

Note the `do ... while(false)` pattern, it is widely used throughout the @bsb firmware.

#### Custom types

Always define custom types explicitly using `typedef`. Avoid using anonymous inline types:

```C
// Not ideal
struct {
    int member_1;
    char member_2[10];
} my_variable;

// Better
typedef struct {
    int member_1;
    char member_2[10];
} MyType;

MyType my_variable;

// ...

// Not ideal
void call_callback(void (*callback)(void*));

// Better
typedef void (*MyCallback)(void* context);

void call_callback(MyCallback callback);
```

#### Object orientation

If a file `my_module.h` describes a class-like object, all of the method functions should have a `my_module_` prefix.

```C
// my_module.h

typedef struct {
    // ...
} MyModule;

typedef struct {
    // ...
} MyModuleDoodad;

void my_module_modulate(MyModule* instance);

void my_module_discombombulate(MyModule* instance, int doodads_count);

void my_module_doodad_transmogrify(MyModuleDoodad* doodad);

```

For object-oriented code, a *this* function parameter should be called `instance`, unless this function operates on a sub-object or a helper type (see above).

Always try hiding as much implementation detail as possible by using opaque pointers:

```C
// my_module.h
// Improved version of the above example

typedef struct MyModule MyModule;

// The only way of getting a pointer to a MyModule instance
MyModule* my_module_alloc(void);

void my_module_modulate(MyModule* instance);

// ...

// my_module.c
#include "my_module.h"

struct MyModule {
    // ....
};

MyModule* my_module_alloc(void) {
    MyModule* instance = malloc(sizeof(MyModule));
    // ....
    return instance;
}
```
