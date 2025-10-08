#ifndef LOG_H
#define LOG_H

#define DEBUG

#ifdef DEBUG
#include <printf.h>
#include <stdio.h>
#define LOG_STR(x) printf("%s\n", x);
#define LOG_STR_NNL(x) printf("%s", x);
#define LOG_CHAR(x) printf("%c\n", x);
#define LOG_CHAR_NNL(x) printf("%c", x);
#define LOG_ERR(x) puts(x);
#else
#define LOG_STR(x)
#define LOG_STR_NNL(x)
#define LOG_CHAR(x)
#define LOG_CHAR_NNL(x)
#define LOG_ERR(x)
#endif

#endif // !LOG_H
