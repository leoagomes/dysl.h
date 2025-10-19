/* This is free and unencumbered software released into the public domain.
 * Anyone is free to copy, modify, publish, use, compile, sell, or
 * distribute this software, either in source code form or as a compiled
 * binary, for any purpose, commercial or non-commercial, and by any
 * means.
 *
 * In jurisdictions that recognize copyright laws, the author or authors
 * of this software dedicate any and all copyright interest in the
 * software to the public domain. We make this dedication for the benefit
 * of the public at large and to the detriment of our heirs and
 * successors. We intend this dedication to be an overt act of
 * relinquishment in perpetuity of all present and future rights to this
 * software under copyright law.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * For more information, please refer to <https://unlicense.org/>. */
#ifndef __DYSL__
#define __DYSL__

#include <stddef.h>
#include <stdint.h>

/* == Version information == */
#define DYSL_VERSION_MAJOR 0
#define DYSL_VERSION_MINOR 1
#define DYSL_VERSION_PATCH 0
#define DYSL_VERSION_STRING "0.1.0"

/* # Configuration
 *
 * You can configure Dysl by defining certain macros before including it, or
 * by defining them all in a configuration file - `dysl-config.h` by default.
 * Dysl will attempt to include this file automatically if it exists. You can
 * change the name of this file by defining the `DYSL_CONFIG_FILE` macro to
 * your desired file name before including Dysl.
 *
 * If no configuration is provided, the default values will be used. Check the
 * header file's "default configuration" section for available configuration
 * options and their default values.
 *
 * If you want to avoid using a configuration file altogether, you can remove
 * the next section that includes it, and define the configuration macros
 * directly below this comment instead.
 */
#ifndef DYSL_CONFIG_FILE
#define DYSL_CONFIG_FILE "dysl-config.h"
#endif /* DYSL_CONFIG_FILE */
#ifdef __has_include
#if __has_include(DYSL_CONFIG_FILE)
#include DYSL_CONFIG_FILE
#else /* __has_include(DYSL_CONFIG_FILE) */
#warning "[dysl] configuration file not found, using defaults"
#endif /* __has_include(DYSL_CONFIG_FILE) */
#else /* __has_include */
#warning "[dysl] cannot check for configuration file, using defaults"
#endif /* __has_include */

/* == Default configuration == */
#ifdef DYSL_CLI
#define DYSL_STDLIB 1
#define DYSL_IMPLEMENTATION 1
#define DYSL_STDIO 1
#endif /* DYSL_CLI */
#ifndef DYSL_STDLIB
#define DYSL_STDLIB 0
#endif /* DYSL_STDLIB */

/* == Configuration-derived includes == */
#if DYSL_STDLIB
#include <stdlib.h>
#endif /* DYSL_STDLIB */
#if DYSL_STDIO
#include <stdio.h>
#endif /* DYSL_STDIO */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/* == Allocator API == */

/** The allocator function type.
 *
 * Similar to Lua's allocator, and the C realloc function, this function
 * should behave as follows:
 *
 * - If `new_size` is non-zero and `ptr` is NULL, a new memory block of size
 * `new_size` should be allocated and a pointer to it returned.
 *
 * - If `new_size` is non-zero and `ptr` is non-NULL, the memory block pointed
 * to by `ptr` should be resized to size `new_size` and a pointer to the
 * resized block returned. The contents of the block should be preserved up to
 * the minimum of the old and new sizes.
 *
 * - If `new_size` is zero and `ptr` is non-NULL, the memory block pointed to
 * by `ptr` should be freed and NULL returned.
 *
 * In any other case (unlikely), or if the allocation/resizing fails,
 * NULL should be returned.
 *
 * @param ud        The user data pointer.
 * @param ptr       The pointer to the previously allocated memory block, or
 *                  NULL if a new block is being allocated.
 * @param old_size  The old size of the memory block, or 0 if a new block is
 *                  being allocated.
 * @param new_size  The new size of the memory block, or 0 if the block is being
 *                  freed.
 */
typedef void*(*dysl_allocator_fn)(
    void* ud, void* ptr,
    size_t old_size, size_t new_size
);

/** The structure that holds the allocator function and its user data. */
struct dysl_allocator {
    void* user_data;      /*< The user data pointer. */
    dysl_allocator_fn fn; /*< The allocator function. */
};

#if DYSL_STDLIB
/** The default allocator that uses the C standard library's malloc,
 * realloc and free functions.
 *
 * @see dysl_allocator_fn
 */
void* dysl_stdlib_allocator_fn(void* ud, void* ptr, size_t os, size_t ns);

/** Returns a `dysl_allocator` that uses the standard library allocator
 * function.
 *
 * @return  A `dysl_allocator` using the standard library functions.
 */
struct dysl_allocator dysl_standard_allocator(void);
#endif /* DYSL_STDLIB */

/* == Dysl API == */

/** The interpreter context. */
struct dysl;

/** Creates a new interpreter context.
 *
 * The returned context should be destroyed with `dysl_destroy()` when no
 * longer needed.
 *
 * @return  A pointer to the new interpreter context, or NULL on failure.
 */
struct dysl* dysl_new(struct dysl_allocator allocator);

/** Destroys an interpreter context.
 *
 * @param dysl  The interpreter context to destroy.
 */
void dysl_destroy(struct dysl* state);
#ifdef __cplusplus
}; /* extern "C" */
#endif /* __cplusplus */

#ifdef DYSL_IMPLEMENTATION
/* == data types == */
// primitive types
typedef int32_t dy_int;
typedef double dy_real;
typedef int32_t dy_bool;
typedef int32_t dy_char;
typedef uint32_t dy_hash_t;

// object types (forward declarations)
struct dy_object;
struct dy_symbol;
struct dy_string;

// tagged union value
typedef uint32_t dy_tag;
struct dy_value {
    dy_tag tag;
    union {
        dy_int integer;
        dy_real real;
        dy_bool boolean;
        dy_char character;
        struct dy_object* object;
        struct dy_string* string;
    } as;
};

// object type definitions
struct dy_object {
    dy_tag tag;
    struct dy_object *previous, *next;
};
static inline void dObj_close(struct dy_object* obj);
static inline void dObj_unlink(struct dy_object* obj);
static inline void dObj_link(struct dy_object* obj, struct dy_object* list);

// symbols
struct dy_symbol {
    struct dy_object header;
    struct dy_symbol* next_in_table;
    size_t length;
    dy_hash_t hash;
    char name[1];
};

// strings
struct dy_string {
    struct dy_object header;
    size_t length;
    char data[1];
};

// garbage collector
struct dy_gc {
    struct dysl_allocator allocator;
    struct dy_object root, gen;
};
#define dGC_allocator(gc) (&((gc)->allocator))
void dGC_init(struct dy_gc* gc, struct dysl_allocator allocator);
static inline void dGC_track(struct dy_gc* gc, struct dy_object* obj);
static inline void dGC_root(struct dy_gc* gc, struct dy_object* obj);
static inline void dGC_unroot(struct dy_gc* gc, struct dy_object* obj);
struct dy_object* dGC_create(struct dy_gc* gc, size_t size, dy_tag tag);

// symbol table
#define DYSL_SYMBOLS_INITIAL_CAPACITY 64
#define DYSL_SYMBOLS_LOAD_FACTOR 0.75
struct dy_symbols {
    struct dy_symbol** buckets;
    size_t count, capacity;
};
/** Initializes the symbol table. */
void dSymbols_init(
    struct dy_symbols* symbols,
    size_t initial_capacity,
    struct dysl_allocator* allocator
);
/** Destroys the symbol table structure, not the symbols themselves. */
void dSymbols_destroy(
    struct dy_symbols* symbols,
    struct dysl_allocator* allocator
);
/** Looks up a symbol in the table, returns its slot. */
struct dy_symbol** dSymbols_lookup(
    struct dy_symbols* symbols,
    const char* name,
    size_t length,
    dy_hash_t hash,
    int* found
);
/** Returns the slot for the interned symbol with the given name, growing the
 * table if necessary.
 *
 * After this call, the table will have counted the new symbol.
 *
 * The caller is responsible for allocating the symbol and placing it in the
 * returned slot.
 */
struct dy_symbol** dSymbols_intern(
    struct dy_symbols* symbols,
    const char* name,
    size_t length,
    dy_hash_t hash,
    struct dysl_allocator* allocator
);
/** Grows or shrinks the symbol table to a reasonable size that accommodates the
 * desired count of symbols. */
void dSymbols_ensure_capacity(
    struct dy_symbols* symbols,
    size_t desired_count,
    struct dysl_allocator* allocator
);
/** Resizes the symbol table to the new capacity. */
void dSymbols_resize(
    struct dy_symbols* symbols,
    size_t new_capacity,
    struct dysl_allocator* allocator
);
/** Returns whether the symbol table should grow to accommodate the desired
 * count of symbols. */
int dSymbols_should_grow(struct dy_symbols* symbols, size_t desired_count);

// global state
struct dy_global {
    struct dy_gc gc;
    struct dy_symbols symbols;
    struct dysl* main_state;
};
#define dGlobal_gc(global) (&((global)->gc))
void dGlobal_init(struct dy_global* global, struct dysl_allocator allocator);

struct dy_stack {
    struct dy_value* base;
    struct dy_value* top;
    size_t size;
};

// interpreter context/state
struct dysl {
    struct dy_global* global;
};
#define dS_gc(state) dGlobal_gc((state)->global)

/* == Allocator API == */
static inline void* dAlloc_alloc(struct dysl_allocator* allocator, size_t size);
static inline void* dAlloc_free(struct dysl_allocator* allocator, void* ptr);
static inline void* dAlloc_realloc(
    struct dysl_allocator* allocator,
    void* ptr,
    size_t old_size,
    size_t new_size
);

/* == utils == */

#define dU_min(a, b) ((a) < (b) ? (a) : (b))
#define dU_max(a, b) ((a) > (b) ? (a) : (b))

/** Clears a chunk of memory (sets to zero) */
static inline void dMem_clear(void* ptr, size_t size) {
    uint8_t* p = (uint8_t*)ptr;
    for (size_t i = 0; i < size; i++)
        p[i] = 0;
}

/** Copies a chunk of memory (from src to dest) */
static inline void dMem_copy(void* dest, const void* src, size_t size) {
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;
    for (size_t i = 0; i < size; i++)
        d[i] = s[i];
}

/** FNV-1a hash function implementation */
static inline dy_hash_t dHash_fnv1a(const void* key, size_t length) {
    const uint8_t* data = (const uint8_t*)key;
    dy_hash_t hash = 2166136261u;
    for (size_t i = 0; i < length; i++) {
        hash ^= data[i];
        hash *= 16777619u;
    }
    return hash;
}
#define dHash_slice(key, length) dHash_fnv1a((key), (length))

/** Compares two memory slices for equality */
static inline int dSlice_equals(
    const void* a,
    size_t a_length,
    const void* b,
    size_t b_length
) {
    if (a_length != b_length)
        return 0;
    const uint8_t* pa = (const uint8_t*)a;
    const uint8_t* pb = (const uint8_t*)b;
    for (size_t i = 0; i < a_length; i++) {
        if (pa[i] != pb[i])
            return 0;
    }
    return 1;
}

/* == Allocator API == */
static inline void* dAlloc_alloc(struct dysl_allocator* allocator, size_t size) {
    return allocator->fn(allocator->user_data, NULL, 0, size);
}
static inline void* dAlloc_free(struct dysl_allocator* allocator, void* ptr) {
    return allocator->fn(allocator->user_data, ptr, 0, 0);
}
static inline void* dAlloc_realloc(
    struct dysl_allocator* allocator,
    void* ptr,
    size_t old_size,
    size_t new_size
) {
    return allocator->fn(allocator->user_data, ptr, old_size, new_size);
}

/* == Object linked list API == */
static inline void dObj_close(struct dy_object* obj) {
    obj->previous = obj->next = obj;
}

static inline void dObj_unlink(struct dy_object* obj) {
    obj->previous->next = obj->next;
    obj->next->previous = obj->previous;
    dObj_close(obj);
}

static inline void dObj_link(struct dy_object* obj, struct dy_object* list) {
    obj->next = list->next;
    obj->previous = list;
    list->next->previous = obj;
    list->next = obj;
}

/* == Symbol table API == */

void dSymbols_init(
    struct dy_symbols* symbols,
    size_t initial_capacity,
    struct dysl_allocator* allocator
) {
    symbols->count = 0;
    symbols->capacity = initial_capacity;
    symbols->buckets = (struct dy_symbol**)dAlloc_alloc(
        allocator,
        sizeof(struct dy_symbol*) * initial_capacity
    );
    for (size_t i = 0; i < initial_capacity; i++)
        symbols->buckets[i] = NULL;
}

void dSymbols_destroy(struct dy_symbols* symbols, struct dysl_allocator* allocator) {
    // destroys only the symbol table structure,
    // the symbols themselves should be GC'd
    dAlloc_free(allocator, symbols->buckets);
    symbols->buckets = NULL;
    symbols->count = 0;
    symbols->capacity = 0;
}

struct dy_symbol** dSymbols_lookup(
    struct dy_symbols* symbols,
    const char* name,
    size_t length,
    dy_hash_t hash,
    int* found
) {
    size_t index = hash % symbols->capacity;
    struct dy_symbol** slot = &(symbols->buckets[index]);
    *found = 0;
    while (*slot != NULL) {
        struct dy_symbol* sym = *slot;
        if (sym->hash == hash &&
            sym->length == length &&
            dSlice_equals(sym->name, sym->length, name, length)) {
            *found = 1;
            break;
        }
        slot = &sym->next_in_table;
    }
    return slot;
}

struct dy_symbol** dSymbols_intern(
    struct dy_symbols* symbols,
    const char* name,
    size_t length,
    dy_hash_t hash,
    struct dysl_allocator* allocator
) {
    int found = 0;
    struct dy_symbol** slot;
    // first lookup
    slot = dSymbols_lookup(symbols, name, length, hash, &found);
    if (found) return slot;
    // not found, create new symbol
    size_t new_count = symbols->count + 1;
    if (dSymbols_should_grow(symbols, new_count)) {
        dSymbols_ensure_capacity(symbols, new_count, allocator);
        // re-lookup after resize
        slot = dSymbols_lookup(symbols, name, length, hash, &found);
    }
    symbols->count = new_count;
    // caller allocates the symbol and places it in *slot
    return slot;
}

void dSymbols_resize(
    struct dy_symbols* symbols,
    size_t new_capacity,
    struct dysl_allocator* allocator
) {
    struct dy_symbol** new_buckets = (struct dy_symbol**)dAlloc_alloc(
        allocator,
        sizeof(struct dy_symbol*) * new_capacity
    );
    if (new_buckets == NULL)
        // Allocation failed, overflow the symbol table until next resize.
        // This keeps things functional at the cost of performance.
        return;
    for (size_t i = 0; i < new_capacity; i++)
        new_buckets[i] = NULL;
    // rehash existing symbols
    for (size_t i = 0; i < symbols->capacity; i++) {
        struct dy_symbol* sym = symbols->buckets[i];
        while (sym != NULL) {
            struct dy_symbol* next_sym = sym->next_in_table;
            size_t new_index = sym->hash % new_capacity;
            sym->next_in_table = new_buckets[new_index];
            new_buckets[new_index] = sym;
            sym = next_sym;
        }
    }
    // free old buckets
    dAlloc_free(allocator, symbols->buckets);
    symbols->buckets = new_buckets;
    symbols->capacity = new_capacity;
}

void dSymbols_ensure_capacity(
    struct dy_symbols* symbols,
    size_t desired_count,
    struct dysl_allocator* allocator
) {
    size_t capacity = symbols->capacity;
    size_t grow_threshold = (size_t)(capacity * DYSL_SYMBOLS_LOAD_FACTOR);
    size_t shrink_threshold = capacity / 4;
    size_t new_capacity = symbols->capacity;
    if (desired_count > grow_threshold) {
        // need to grow
        while (new_capacity * DYSL_SYMBOLS_LOAD_FACTOR < desired_count) {
            new_capacity *= 2;
        }
    } else if (desired_count < shrink_threshold &&
               capacity > DYSL_SYMBOLS_INITIAL_CAPACITY) {
        // need to shrink
        new_capacity = capacity / 2;
        new_capacity = dU_max(new_capacity, DYSL_SYMBOLS_INITIAL_CAPACITY);
        while (new_capacity > DYSL_SYMBOLS_INITIAL_CAPACITY &&
               desired_count < (size_t)(new_capacity / 4)) {
            new_capacity /= 2;
        }
    }
    dSymbols_resize(symbols, new_capacity, allocator);
}

int dSymbols_should_grow(struct dy_symbols* symbols, size_t desired_count) {
    size_t grow_threshold = (size_t)(symbols->capacity * DYSL_SYMBOLS_LOAD_FACTOR);
    return desired_count > grow_threshold;
}

/* == Garbage collector API == */
void dGC_init(struct dy_gc* gc, struct dysl_allocator allocator) {
    gc->allocator = allocator;
    dObj_close(&gc->root);
    dObj_close(&gc->gen);
}

static inline void dGC_track(struct dy_gc* gc, struct dy_object* obj) {
    dObj_link(obj, &gc->gen);
}

static inline void dGC_root(struct dy_gc* gc, struct dy_object* obj) {
    dObj_link(obj, &gc->root);
}

static inline void dGC_unroot(struct dy_gc* gc, struct dy_object* obj) {
    dObj_unlink(obj);
}

struct dy_object* dGC_create(struct dy_gc* gc, size_t size, dy_tag tag) {
    struct dy_object* obj = (struct dy_object*)dAlloc_alloc(dGC_allocator(gc),
                                                            size);
    if (obj == NULL)
        return NULL;
    obj->tag = tag;
    // dObj_close(obj); // unnecessary
    dGC_track(gc, obj);
    return obj;
}

/* == Global state API == */
void dGlobal_init(struct dy_global* global, struct dysl_allocator allocator) {
    dGC_init(&global->gc, allocator);
    dSymbols_init(&global->symbols, DYSL_SYMBOLS_INITIAL_CAPACITY, &allocator);
}

/* == Dysl API == */
struct dysl* dysl_new(struct dysl_allocator allocator) {
    struct dysl* D = (struct dysl*)dAlloc_alloc(&allocator, sizeof(*D));
    if (D == NULL)
        return NULL;
    D->global = (struct dy_global*)dAlloc_alloc(&allocator, sizeof(*D->global));
    if (D->global == NULL) {
        dAlloc_free(&allocator, D);
        return NULL;
    }
    dGlobal_init(D->global, allocator);
    D->global->main_state = D;
    return D;
}

void dysl_destroy(struct dysl* state) {
    struct dysl_allocator* allocator = &state->global->gc.allocator;
    dAlloc_free(allocator, state->global);
    dAlloc_free(allocator, state);
}

/* == Standard allocator implementation == */
#if DYSL_STDLIB
void* dysl_stdlib_allocator_fn(void* _ud, void* ptr, size_t _os, size_t ns) {
    (void)_ud; // unused
    (void)_os; // unused
    // allocate new block
    if (ptr == NULL && ns > 0)
        return malloc(ns);
    // free block
    if (ptr != NULL && ns == 0) {
        free(ptr);
        return NULL;
    }
    // resize block
    if (ptr != NULL && ns > 0)
        return realloc(ptr, ns);
    // invalid case
    return NULL;
}

struct dysl_allocator dysl_standard_allocator(void) {
    return ((struct dysl_allocator){
        .user_data = NULL,
        .fn = dysl_stdlib_allocator_fn
    });
}
#endif /* DYSL_STDLIB */
#endif /* DYSL_IMPLEMENTATION */

/* == Command-line interface implementation == */
#ifdef DYSL_CLI
#include <string.h>

void usage(const char* program_name);
void version(void);

int main(int argc, const char* argv[]) {
    const char* program_name = argv[0];
    const char* file_name = NULL;
    // parse command-line arguments
    int arg_index = 1;
    for (arg_index = 1; arg_index < argc; arg_index++) {
        const char* arg = argv[arg_index];
        if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0) {
            usage(program_name);
            return 0;
        } else if (strcmp(arg, "-v") == 0 || strcmp(arg, "--version") == 0) {
            version();
            return 0;
        } else if (arg[0] == '-') {
            printf("Unknown option: %s\n", arg);
            usage(program_name);
            return 1;
        } else {
            // assume it's the script file
            break;
        }
    }
    // if a script file is provided, use it
    if (arg_index < argc) {
        file_name = argv[arg_index];
    }
    // temporarily: fail if no script file is provided
    // TODO: REPL
    if (file_name == NULL) {
        printf("No script file provided.\n");
        usage(program_name);
        return 1;
    }
    struct dysl* dysl = dysl_new(dysl_standard_allocator());
    if (dysl == NULL) {
        printf("Failed to create dysl interpreter.\n");
        return 1;
    }

    dysl_destroy(dysl);
    return 0;
}

void usage(const char* program_name) {
    printf("Usage: %s [options] [script]\n", program_name);
    printf("Options:\n");
    printf("  -h, --help      Show this help message and exit\n");
    printf("  -v, --version   Show version information and exit\n");
}

void version(void) {
    printf("dysl version %s\n", DYSL_VERSION_STRING);
}
#endif /* DYSL_CLI */
#endif /* __DYSL__ */
