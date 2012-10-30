#ifndef SANITIZE_COMMON_H_INCLUDED
#define SANITIZE_COMMON_H_INCLUDED

typedef void (*free_function_t)(void*);
typedef int (*less_function_t)(const void* o1, const void *o2);

int ptr_less(const void* o1, const void *o2);

unsigned hash_function(const char *str, size_t len);
size_t align_size_8(size_t num);
size_t align_size_64(size_t num);

#endif

