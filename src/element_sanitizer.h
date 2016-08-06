#ifndef SANITIZE_ELEMENT_SANITIZER_H_INCLUDED
#define SANITIZE_ELEMENT_SANITIZER_H_INCLUDED

typedef struct ElementSanitizer ElementSanitizer;

ElementSanitizer *element_sanitizer_new(void);
void element_sanitizer_free(ElementSanitizer *es);

void element_sanitizer_add_regex(ElementSanitizer *es, const char *attribute, const char *re, int inverted);
int element_sanitizer_is_valid(ElementSanitizer *es, const char *attribute, const char *value);

#endif

