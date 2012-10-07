#ifndef SANITIZE_H_INCLUDED
#define SANITIZE_H_INCLUDED

#include "mode.h"

char *sanitize(const char *html, struct sanitize_mode *mode);

#endif

