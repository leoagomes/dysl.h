## dysl

A **dy**namic **s**tack **l**anguage, header only, no external dependencies, in C.

Dysl (pronouced diesel) is:
- **Header only.** Easy to build or add to your project.
- **Extensible small core.** With a simple C API.
- **Batteries on the side.** So you add only the features you want.
- **No external dependencies.** Only the C standard library, if you want it.
- **Delightfully documented.** In three simple files.
- **Public domain.** Do with it as you please.

## Getting Started

All you need is a C99 compatible compiler.

```sh
gcc -DDYSL_CLI -xc dysl.h -o dysl
```

### Embedding

Adding Dysl to your C/C++ app is as simple as:
1. Adding a copy of `dysl.h` to your project;
2. Configuring it to your liking;
3. `#include` it in your project;
4. `#define DYSL_IMPLEMENTATION` somewhere in your `.c` (or C++) files.

```c
// wherever you need it
#include "dysl.h"

// in one file, possibly dysl.c -- but **only once**!
#define DYSL_IMPLEMENTATION
#include "dysl.h"
```

Read the [doc on embedding](./docs/embedding.md), it's super short.

### Hello, world

```
"hello, world" print
```

## Documentation

You're already reading through the bulk of it, but:
- `dysl.h` includes great inline documentation,
- there's a short but complete language [reference doc](./docs/reference.md), and
- [`embedding.md`](./docs/embedding.md) will get you through the main uses of it.

## License

Dysl is released under the Unlicense. Do with it as you please.

