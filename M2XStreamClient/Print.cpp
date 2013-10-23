#include "Print.h"
#include "mbed.h"

#include <stdio.h>
#include <string.h>

size_t Print::write(const uint8_t* buf, size_t size) {
  size_t ret = 0;
  while (size--) {
    ret += write(*buf++);
  }
  return ret;
}

size_t Print::print(const char* s) {
  return write((const uint8_t*)s, strlen(s));
}

size_t Print::print(char c) {
  return write(c);
}

size_t Print::print(int n) {
  return print((long) n);
}

size_t Print::print(long n) {
  char buf[8 * sizeof(long) + 1];
  snprintf(buf, sizeof(buf), "%ld", n);
  return print(buf);
}

// Digits are ignored for now
size_t Print::print(double n, int digits) {
  char buf[65];
  snprintf(buf, sizeof(buf), "%g", n);
  return print(buf);
}

size_t Print::println(const char* s) {
  return print(s) + println();
}

size_t Print::println(char c) {
  return print(c) + println();
}

size_t Print::println(int n) {
  return print(n) + println();
}

size_t Print::println(long n) {
  return print(n) + println();
}

size_t Print::println(double n, int digits) {
  return print(n, digits) + println();
}

size_t Print::println() {
  return print('\r') + print('\n');
}
