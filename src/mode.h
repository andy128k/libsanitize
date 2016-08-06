#ifndef SANITIZE_MODE_H_INCLUDED
#define SANITIZE_MODE_H_INCLUDED

#include "array.h"
#include "dict.h"
#include "element_sanitizer.h"
#include "value_checker.h"
#include "quarks.h"

/* quarks */
extern const char *Q_WHITESPACE;

void mode_init_quarks(void);

/* mode */

struct sanitize_mode
{
  int allow_comments;
  Dict *elements;               /* tag name --> element sanitizer */
  Dict *delete_elements;        /* set */
  Dict *rename_elements;
};

struct sanitize_mode *mode_new(void);
struct sanitize_mode *mode_load(const char *filename);
struct sanitize_mode *mode_memory(const char *data);
void mode_free(struct sanitize_mode *mode);

#endif

