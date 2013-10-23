#ifndef Print_h
#define Print_h

#include <stddef.h>
#include <stdint.h>

class Print {
public:
  size_t print(const char* s);
  size_t print(char c);
  size_t print(int n);
  size_t print(long n);
  size_t print(double n, int digits = 2);

  size_t println(const char* s);
  size_t println(char c);
  size_t println(int n);
  size_t println(long n);
  size_t println(double n, int digits = 2);
  size_t println();

  virtual size_t write(uint8_t c) = 0;
  virtual size_t write(const uint8_t* buf, size_t size);
};

#endif
