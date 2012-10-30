#include <stdlib.h>
#include <string.h>
#include <regex.h>

#include "value_checker.h"
#include "array.h"

struct Check
{
#ifndef NDEBUG
  char *re;
#endif
  regex_t preg;
  int inverted;
};

static void free_check(struct Check *ch)
{
#ifndef NDEBUG
  free(ch->re);
#endif
  regfree(&ch->preg);
  free(ch);
}

/* ValueChecker is an alias to Array */

ValueChecker *value_checker_new(void)
{
  return (ValueChecker *)array_new((free_function_t)free_check);
}

void value_checker_free(ValueChecker *vc)
{
  array_free((Array *)vc);
}

void value_checker_add_regex(ValueChecker *vc, const char *re, int inverted)
{
  if (re && *re)
    {
      struct Check *ch = malloc(sizeof(struct Check));
#ifndef NDEBUG
      ch->re = strdup(re);
#endif
      regcomp(&ch->preg, re, REG_EXTENDED | REG_ICASE | REG_NOSUB);
      ch->inverted = inverted;
      array_append((Array *)vc, ch);
    }
  else
    {
      array_clean((Array *)vc);
    }
}

int value_checker_check(ValueChecker *vc, const char *value)
{
  size_t i, size;

  if (!vc)
    return 0;

  size = ((Array *)vc)->size;
  if (!size)
    return 1;

  for (i = 0; i < size; ++i)
    {
      struct Check *check = ((Array *)vc)->items[i];

      int r = !regexec(&check->preg, value, 0, NULL, 0);
      if (check->inverted)
	r = !r;
      if (r)
	return 1;
    }

  return 0;
}

