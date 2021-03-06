#ifndef ARRAY_H_INCLUDED
#define ARRAY_H_INCLUDED

#include <stdarg.h>
#include "common.h"

typedef struct Array Array;

struct Array
{
  void **items;
  size_t size;
  size_t allocated;
  free_function_t free_item_func;
};

Array *array_new(free_function_t free_item_func);
void array_free(Array *arr);
void array_reserve(Array *arr, size_t count);
void array_append(Array *arr, void *item);
void array_insert(Array *arr, size_t index, void *item);
void array_clean(Array *arr);

typedef int (*array_item_predicate_t)(const void *item, const void *user_data);
void *array_find(Array *arr, array_item_predicate_t pred, const void *user_data);
void *array_find_not(Array *arr, array_item_predicate_t pred, const void *user_data);

// for sorted arrays
size_t array_lower_bound(Array *arr, const void *value, less_function_t less);

#endif

