# Localization data

## Subdirectories
  - Localization data from the `internal` directory is linked directly into the
    firmware. You should use the `L10nSourceFlash` option in your code.
  - Files from the `external` directory are converted and put on the eMMC
    storage. For those ones, use `L10nSourceStorage`.

Apart from the `L10nSourceX` option that you provide to `l10n_context_open`,
the API for working with both formats is the same.

## File format
The files should be named `<app_name>.csv`. The first row is interpreted as a
table header. There are two required columns: `key` and `en-US`. With the
exception of `key`, column names correspond to locale names. `key` is used to
look up a localization template within the code.

Here's an example:
```csv
key,en-US,ru-RU
greeting,Hello,Привет
world,World,Мир
```

Newlines are allowed but should be quoted. Unfortunately, not all CSV tools
correctly interpret such files.

```csv
key,en-US,ru-RU
example1,"This
is
an
example","Это
пример"
```

## Example
`internal/my_app.csv` _or_ `external/my_app.csv`, but not both:
```
key,en-US,ru-RU
demo.test,"Hello, World!","Привет, Мир!"
```

```c
#include <l10n/l10n.h>
#include <l10n_keys/my_app.h> // generated at build time

L10nSrv* l10n_srv = /* ... */; // most likely `furi_record_open`

// for `internal`:
#define APP_ID "my_app"
L10nContext* l10n = l10n_context_open(l10n_srv, APP_ID, L10nSourceFlash);

// for `external`:
#define MY_APP_ASSETS_PATH(path) EXT_PATH("apps_assets/my_app") "/" path
L10nContext* l10n = l10n_context_open(l10n_srv, MY_APP_ASSETS_PATH("l10n"), L10nSourceStorage);

// unified interface after acquiring the handle:
const char* translation = l10n_get(l10n, L10N_KEY_MY_APP_DEMO_TEST);

l10n_context_close(l10n);
```

Also read the detailed documentation for the `L10n` service.
