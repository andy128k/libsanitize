#ifndef SANITIZE_QUARKS_H_INCLUDED
#define SANITIZE_QUARKS_H_INCLUDED

void init_quarks(void);
void free_quarks(void);

const char *quark(const char *str);
int is_quark(char *value);

void qfree(void *mem);

#endif

