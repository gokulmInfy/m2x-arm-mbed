#ifndef UTILITY_H_
#define UTILITY_H_

#include "mbed.h"
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

void delay(int ms);
char* strdup(const char* s);

#ifdef __cplusplus
}
#endif

#endif