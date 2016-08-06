#ifndef SANITIZE_ELEMENT_SANITIZER_H_INCLUDED
#define SANITIZE_ELEMENT_SANITIZER_H_INCLUDED

#include "dict.h"

typedef struct ElementSanitizer ElementSanitizer;

ElementSanitizer *element_sanitizer_new(void);
void element_sanitizer_free(ElementSanitizer *es);

void element_sanitizer_add_regex(ElementSanitizer *es, const char *attribute, const char *re, int inverted);
int element_sanitizer_is_valid(ElementSanitizer *es, const char *attribute, const char *value);

void element_sanitizer_add_mandatory_attribute(ElementSanitizer *es, const char *attribute, const char *value);
Dict *element_sanitizer_get_mandatory_attributes(ElementSanitizer *es);

#endif

