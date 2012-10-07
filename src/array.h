#ifndef ARRAY_H_INCLUDED
#define ARRAY_H_INCLUDED

#include <stdarg.h>

typedef struct _Array Array;

typedef void (*array_item_free_t)(void*);

struct _Array
{
  void **items;
  size_t size;
  size_t allocated;
  array_item_free_t free_item_func;
};

Array *array_new(array_item_free_t free_item_func);
Array *array_newv(array_item_free_t free_item_func, ...);
void array_free(Array *arr);
void array_reserve(Array *arr, size_t count);
void array_append(Array *arr, void *item);
void array_append_va(Array *arr, va_list args);
void array_appendv(Array *arr, ...);

typedef int (*array_item_predicate_t)(const void *item, const void *user_data);
void *array_find(Array *arr, array_item_predicate_t pred, const void *user_data);
void *array_find_not(Array *arr, array_item_predicate_t pred, const void *user_data);

#endif

