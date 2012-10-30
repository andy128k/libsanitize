#include <stdlib.h>
#include <string.h>

#include <stdio.h>


#include "common.h"
#include "array.h"

#define HASH_SIZE (97)

static Array *quarks = NULL;
static Array *quarks_index = NULL;

void init_quarks(void)
{
  size_t index;
  
  if (quarks)
    return;

  quarks = array_new((free_function_t)array_free);
  array_reserve(quarks, HASH_SIZE);
  for (index = 0; index < HASH_SIZE; ++index)
    array_append(quarks, array_new((free_function_t)free));

  quarks_index = array_new(NULL);
}

void free_quarks(void)
{
  array_free(quarks);
  array_free(quarks_index);
  quarks = quarks_index = NULL;
}

const char *quark(const char *str)
{
  Array *bucket;
  char *value;
  size_t index;

  init_quarks();

  bucket = quarks->items[hash_function(str, 0) % HASH_SIZE];
  value = array_find_not(bucket, (array_item_predicate_t)strcmp, str);

  if (!value)
    {
      value = strdup(str);
      array_append(bucket, value);

      index = array_lower_bound(quarks_index, value, ptr_less);
      array_insert(quarks_index, index, value);
    }

  return value;
}

int is_quark(char *value)
{
  if (!quarks)
    return 0;

  size_t lb = array_lower_bound(quarks_index, value, ptr_less);
  return lb < quarks_index->size && value == quarks_index->items[lb];
}

void qfree(void *mem)
{
  if (mem && !is_quark(mem))
    free(mem);
}

