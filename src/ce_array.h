// Capsian-Engine - Ougi Washi

#ifndef CE_ARRAY_H
#define CE_ARRAY_H

#include <string.h>
#include <assert.h>
#include "ce_types.h"

// This approach is inspired by arena allocators but per array instead of being block-based to avoid fragmentation and wasted memory
// while offering a simple array handling interface.

#define CE_DEFINE_ARRAY(_type, _array, _size) \
    typedef struct { \
        _type data[_size]; \
        sz size; \
    } _array; \
    static void _array##_init(_array* array) { \
        memset(array, 0, sizeof(_array)); \
        array->size = 0; \
    } \
    static _type* _array##_increment(_array* array) { \
        if (array->size == _size) { \
            return NULL; \
        } \
        array->size++; \
        return &array->data[array->size - 1]; \
    } \
    static _type* _array##_add(_array* array, _type value) { \
        _type* new_element = _array##_increment(array); \
        *new_element = value; \
        return new_element; \
    } \
    static void _array##_remove(_array* array, const sz index) { \
        array->size--; \
        memmove(&array->data[index], &array->data[index + 1], sizeof(array->data[index]) * (array->size - index)); \
    } \
    static _type* _array##_get(_array* array, const sz index) { \
        ce_assert(index >= 0 && index < array->size); \
        return &array->data[index]; \
    } \
    static void _array##_set(_array* array, const sz index, _type* value) { \
        ce_assert(index < array->size); \
        array->data[index] = *value; \
    } \
    static void _array##_clear(_array* array) { \
        memset(array->data, 0, sizeof(_type) * array->size); \
        array->size = 0; \
    } \
    static sz _array##_get_size(const _array* array) { \
        return array->size; \
    } \

#define ce_foreach(_array_type, _array, _it) \
    for (sz _it = 0; _it < _array_type##_get_size(&_array); _it++)


#endif // CE_ARRAY_H
