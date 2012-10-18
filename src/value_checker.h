#ifndef SANITIZE_VALUE_CHECKER_H_INCLUDED
#define SANITIZE_VALUE_CHECKER_H_INCLUDED

typedef struct ValueChecker ValueChecker;

ValueChecker *value_checker_new(void);
void value_checker_free(ValueChecker *vc);
void value_checker_add_regex(ValueChecker *vc, const char *re, int inverted);
int value_checker_check(ValueChecker *vc, const char *value);

#endif

