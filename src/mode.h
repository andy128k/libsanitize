#ifndef SANITIZE_MODE_H_INCLUDED
#define SANITIZE_MODE_H_INCLUDED

#include <regex.h>
#include "array.h"

struct attribute
{ 
  char *name;
  char *value;
  regex_t preg;
  int inverted;
};

struct attribute *attribute_new(const char *name, const char *value);
void attribute_free(struct attribute *attr);
int attribute_check(struct attribute *attr, const char *name, const char *value);

struct element
{
  char *tagname;
  Array *attributes;
};

struct element *element_new(const char *tagname);
void element_free(struct element *el);

struct sanitize_mode
{
  int allow_comments;
  Array *elements;
  Array *common_attributes;
  Array *whitespace_elements;
};

struct sanitize_mode *mode_new(int allow_comments,
			       Array *elements,
			       Array *common_attributes,
			       Array *whitespace_elements);

struct sanitize_mode *mode_load(const char *filename);

void mode_free(struct sanitize_mode *mode);

struct element *mode_find_element(struct sanitize_mode *mode, const char *tagname);

#endif

