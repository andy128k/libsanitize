#ifndef SANITIZE_DICT_H_INCLUDED
#define SANITIZE_DICT_H_INCLUDED

#include "types.h"
 
typedef struct Dict Dict;

Dict *dict_new(free_function_t value_free_func);
void dict_free(Dict *dict);
void dict_replace(Dict *dict, const char *key, void *value);
void dict_replacen(Dict *dict, const char *key, size_t key_len, void *value);
void *dict_get(Dict *dict, const char *key);
void *dict_getn(Dict *dict, const char *key, size_t key_len);

#endif

