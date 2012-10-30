#ifndef SANITIZE_MODE_H_INCLUDED
#define SANITIZE_MODE_H_INCLUDED

#include "array.h"
#include "dict.h"
#include "value_checker.h"
#include "quarks.h"

/* quarks */
extern const char *Q_WHITESPACE;

void mode_init_quarks(void);

/* mode */

struct sanitize_mode
{
  int allow_comments;
  Dict *elements;               /* tag name --> dict of attributes (attr name --> value checker) */
  Dict *common_attributes;      /* attr name --> value checker */
  Dict *rename_elements;
};

struct sanitize_mode *mode_new(void);
struct sanitize_mode *mode_load(const char *filename);
void mode_free(struct sanitize_mode *mode);

#endif

