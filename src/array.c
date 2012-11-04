#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "array.h"

Array *array_new(free_function_t free_item_func)
{
  Array *arr = malloc(sizeof(struct Array));
  arr->allocated = 0;
  arr->size = 0;
  arr->items = NULL;
  arr->free_item_func = free_item_func;
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
      arr->allocated = align_size_64(arr->size + count);
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

void array_insert(Array *arr, size_t index, void *item)
{
  if (!arr)
    return;

  array_reserve(arr, 1);
  memmove(&arr->items[index + 1],
          &arr->items[index],
          arr->size - index);
  arr->size++;

  if (arr->free_item_func && arr->items[index])
    arr->free_item_func(arr->items[index]);

  arr->items[index] = item;
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

size_t array_lower_bound(Array *arr, const void *value, less_function_t less)
{
  size_t first = 0;
  size_t count = arr->size;

  while (count > 0)
  {
    const size_t half = count / 2;

    if (less(arr->items[first + half], value))
      {
        first += half + 1;
        count -= half + 1;
      }
    else
      {
        count = half;
      }
  }
  return first;
}

