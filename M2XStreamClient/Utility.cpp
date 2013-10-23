#include "Utility.h"

void delay(int ms) {
  wait_ms(ms);
}

char* strdup(const char* s) {
  char* ret = (char*) malloc(strlen(s) + 1);
  if (ret == NULL) { return ret;}
  return strcpy(ret, s);
}
