// Capsian-Engine - Ougi Washi

#ifndef CE_TYPES_H
#define CE_TYPES_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>

typedef bool b8;
typedef int8_t i8;
typedef uint8_t u8;
typedef int16_t i16;
typedef uint16_t u16;
typedef int32_t i32;
typedef uint32_t u32;
typedef int64_t i64;
typedef uint64_t u64;
typedef float f32;
typedef double f64;
typedef char c8;
typedef unsigned char uc8;
typedef size_t sz;

#ifndef RESOURCES_DIR
#  error "RESOURCES_DIR not defined!"
#endif

#define MAX_PATH_LENGTH 256

#define ce_assert(expr) if (!(expr)) { fprintf(stderr, "Assertion failed: %s\n", #expr); assert(0); }
#define ce_assertf(expr, msg) if (!(expr)) { fprintf(stderr, msg); assert(0); }

#endif // CE_TYPES_H
