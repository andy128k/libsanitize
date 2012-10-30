#include <stdlib.h>
#include <string.h>
#include "dict.h"

#define HASH_SIZE (97)

struct HashValue
{
  char *key;
  void *value;
};

struct Bucket
{
  unsigned count;
  unsigned allocated;
  struct HashValue *values;
};

struct Dict
{
  free_function_t value_free;
  struct Bucket data[HASH_SIZE];
};

Dict *dict_new(free_function_t value_free_func)
{
  Dict *dict = malloc(sizeof(struct Dict));
  memset(dict, 0, sizeof(struct Dict));
  dict->value_free = value_free_func;
  return dict;
}

void dict_free(Dict *dict)
{
  if (dict)
    {
      unsigned i, j;

      for (i = 0; i < HASH_SIZE; ++i)
	{
	  struct Bucket *bucket = &dict->data[i];

	  for (j = 0; j < bucket->count; ++j)
	    free(bucket->values[j].key);

	  if (dict->value_free)
	    for (j = 0; j < bucket->count; ++j)
	      if (bucket->values[j].value)
		dict->value_free(bucket->values[j].value);

	  free(dict->data[i].values);
	}

      free(dict);
    }
}

static int dict_find(Dict *dict, const char *key, size_t key_len, struct Bucket **bucket, unsigned *index)
{
  const unsigned hash = hash_function(key, key_len);
  unsigned i;

  *bucket = &dict->data[hash % HASH_SIZE];

  for (i = 0; i < (*bucket)->count; ++i)
    {
      if (!strncmp((*bucket)->values[i].key, key, key_len))
	{
	  *index = i;
	  return 1;
	}
    }
  
  *index = -1;
  return 0;
}

void dict_replace(Dict *dict, const char *key, void *value)
{
  dict_replacen(dict, key, strlen(key), value);
}

void dict_replacen(Dict *dict, const char *key, size_t key_len, void *value)
{
  if (!dict || !key)
    return;

  struct Bucket *bucket;
  unsigned index;

  if (dict_find(dict, key, key_len, &bucket, &index))
    {
      void *old_value = bucket->values[index].value;
      if (dict->value_free && old_value)
	dict->value_free(old_value);
      bucket->values[index].value = value;
    }
  else
    {
      /* grow bucket */
      if (bucket->count + 1 >= bucket->allocated)
	{
	  bucket->allocated = align_size_8(bucket->count + 1);
	  bucket->values = realloc(bucket->values, bucket->allocated * sizeof(struct HashValue));
	}

      bucket->values[bucket->count].key = strdup(key);
      bucket->values[bucket->count].value = value;
      ++bucket->count;
    }
}

void *dict_get(Dict *dict, const char *key)
{
  return dict_getn(dict, key, strlen(key));
}

void *dict_getn(Dict *dict, const char *key, size_t key_len)
{
  struct Bucket *bucket;
  unsigned index;

  if (dict_find(dict, key, key_len, &bucket, &index))
    {
      return bucket->values[index].value;
    }
  else
    {
      return NULL;
    }
}

