#include <stdlib.h>
#include <string.h>

#include "element_sanitizer.h"

#include "value_checker.h"
#include "dict.h"

struct ElementSanitizer {
  Dict *attributes;              /* attr name --> value checker */
};

ElementSanitizer *element_sanitizer_new(void)
{
  ElementSanitizer *es = malloc(sizeof(struct ElementSanitizer));
  es->attributes = dict_new((free_function_t)value_checker_free);
  return es;
}

void element_sanitizer_free(ElementSanitizer *es)
{
  if (!es)
    return;
  dict_free(es->attributes);
  free(es);
}

void element_sanitizer_add_regex(ElementSanitizer *es, const char *attribute, const char *re, int inverted)
{
  ValueChecker *vc;

  vc = dict_get(es->attributes, attribute);
  if (!vc)
    {
      vc = value_checker_new();
      dict_replace(es->attributes, attribute, vc);
    }

  value_checker_add_regex(vc, re, inverted);
}

int element_sanitizer_is_valid(ElementSanitizer *es, const char *attribute, const char *value)
{
  ValueChecker *vc;

  vc = dict_get(es->attributes, attribute);
  return value_checker_check(vc, value);
}

