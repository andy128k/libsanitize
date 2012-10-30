#include <stdlib.h>
#include <string.h>

int ptr_less(const void* o1, const void *o2)
{
  return o1 < o2;
}

unsigned hash_function(const char *str, size_t len)
{
  unsigned hash;
  size_t i;

  if (!len)
    len = strlen(str);

  hash = 0;
  for (i = 0; i < len; ++i)
    hash = str[i] + 31 * hash;
  return hash;
}

size_t align_size_8(size_t num)
{
  return (num + 7) / 8 * 8;
}

size_t align_size_64(size_t num)
{
  return (num + 63) / 64 * 64;
}

