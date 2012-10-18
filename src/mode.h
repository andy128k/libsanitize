#ifndef SANITIZE_MODE_H_INCLUDED
#define SANITIZE_MODE_H_INCLUDED

#include "array.h"
#include "dict.h"
#include "value_checker.h"

struct sanitize_mode
{
  int allow_comments;
  Dict *elements;               /* tag name --> dict of attributes (attr name --> value checker) */
  Dict *common_attributes;      /* attr name --> value checker */
  Array *whitespace_elements;
};

struct sanitize_mode *mode_new(int allow_comments,
                               Dict *elements,
			       Dict *common_attributes,
                               Array *whitespace_elements);

struct sanitize_mode *mode_load(const char *filename);

void mode_free(struct sanitize_mode *mode);

#endif

