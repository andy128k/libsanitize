#include <stdlib.h>
#include <stdarg.h>
#include "array.h"

static inline size_t align_size(size_t num)
{
  return (num + 63) / 64 * 64;
}

Array *array_new(free_function_t free_item_func)
{
  Array *arr = malloc(sizeof(struct Array));
  arr->allocated = 0;
  arr->size = 0;
  arr->items = NULL;
  arr->free_item_func = free_item_func;
  return arr;
}

Array *array_newv(free_function_t free_item_func, ...)
{
  Array *arr = array_new(free_item_func);
  va_list args;

  va_start(args, free_item_func);
  array_append_va(arr, args);
  va_end(args);
  return arr;
}

void array_free(Array *arr)
{
  if (arr)
    {
      array_clean(arr);
      if (arr->items)
	free(arr->items);
      free(arr);
    }
}

void array_reserve(Array *arr, size_t count)
{
  if (!arr || !count)
    return;
  if (arr->size + count >= arr->allocated)
    {
      arr->allocated = align_size(arr->size + count);
      arr->items = realloc(arr->items, arr->allocated * sizeof(void*));
    }
}

void array_append(Array *arr, void *item)
{
  if (!arr)
    return;
  array_reserve(arr, 1);
  arr->items[arr->size] = item;
  arr->size++;
}

void array_append_va(Array *arr, va_list args)
{
  void *arg;
  
  if (!arr)
    return;
  
  while (1)
    {
      arg = va_arg(args, void *);
      if (arg)
	array_append(arr, arg);
      else
	break;
    }
}

void array_appendv(Array *arr, ...)
{
  va_list args;

  if (!arr)
    return;

  va_start(args, arr);
  array_append_va(arr, args);
  va_end(args);
}

void array_clean(Array *arr)
{
  if (!arr || !arr->size)
    return;

  if (arr->free_item_func)
    {
      size_t i;
      for (i = 0; i < arr->size; ++i)
	if (arr->items[i])
	  arr->free_item_func(arr->items[i]);
    }

  arr->size = 0;
}

void *array_find(Array *arr, array_item_predicate_t pred, const void *user_data)
{
  size_t i;

  if (!arr)
    return NULL;

  for (i = 0; i < arr->size; ++i)
    {
      void *item = arr->items[i];
      if (pred(item, user_data))
        return item;
    }
  return NULL;
}

void *array_find_not(Array *arr, array_item_predicate_t pred, const void *user_data)
{
  size_t i;

  if (!arr)
    return NULL;

  for (i = 0; i < arr->size; ++i)
    {
      void *item = arr->items[i];
      if (!pred(item, user_data))
        return item;
    }
  return NULL;
}

