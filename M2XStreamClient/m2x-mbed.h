#include "mbed.h"
#define USER_AGENT "User-Agent: M2X Mbed Client/" M2X_VERSION

#ifdef DEBUG
#define DBG(fmt_, data_) printf((fmt_), (data_))
#define DBGLN(fmt_, data_) printf((fmt_), (data_)); printf("\n")
#define DBGLNEND printf("\n")
#endif  /* DEBUG */

class M2XTimer {
public:
  void start() { _timer.start(); }

  unsigned long read_ms() {
    // In case of a timestamp overflow, we reset the server timestamp recorded.
    // Notice that unlike Arduino, mbed would overflow every 30 minutes,
    // so if 2 calls to this API are more than 30 minutes apart, we are
    // likely to run into troubles. This is a limitation of the current
    // mbed platform. However, we argue that it might be a rare case that
    // 2 calls to this are 30 minutes apart. In most cases, we would call
    // this API every few seconds or minutes, this won't be a huge problem.
    // However, if you have a use case that would require 2 intervening
    // calls to this be 30 minutes apart, you might want to leverage a RTC
    // clock instead of the simple ticker here, or call TimeService::reset()
    // before making an API call to sync current time.
    return _timer.read_ms();
  }
private:
  Timer _timer;
};

#include "TCPSocketConnection.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

void delay(int ms)
{
  wait_ms(ms);
}

char* strdup(const char* s)
{
  char* ret = (char*) malloc(strlen(s) + 1);
  if (ret == NULL) { return ret;}
  return strcpy(ret, s);
}

#ifdef __cplusplus
}
#endif

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

/*
 * TCP Client
 */
class Client : public Print {
public:
  Client();
  ~Client();

  virtual int connect(const char *host, uint16_t port);
  virtual size_t write(uint8_t);
  virtual size_t write(const uint8_t *buf, size_t size);
  virtual int available();
  virtual int read();
  virtual void flush();
  virtual void stop();
  virtual uint8_t connected();
private:
  virtual int read(uint8_t *buf, size_t size);
  void _fillin(void);
  uint8_t _inbuf[128];
  uint8_t _incnt;
  void _flushout(void);
  uint8_t _outbuf[128];
  uint8_t _outcnt;
  TCPSocketConnection _sock;
};

Client::Client() : _incnt(0), _outcnt(0), _sock() {
    _sock.set_blocking(false, 1500);
}

Client::~Client() {
}

int Client::connect(const char *host, uint16_t port) {
  return _sock.connect(host, port) == 0;
}

size_t Client::write(uint8_t b) {
  return write(&b, 1);
}

size_t Client::write(const uint8_t *buf, size_t size) {
  size_t cnt = 0;
  while (size) {
    int tmp = sizeof(_outbuf) - _outcnt;
    if (tmp > size) tmp = size;
    memcpy(_outbuf + _outcnt, buf, tmp);
    _outcnt += tmp;
    buf += tmp;
    size -= tmp;
    cnt += tmp;
    // if no space flush it
    if (_outcnt == sizeof(_outbuf))
        _flushout();
  }
  return cnt;
}

void Client::_flushout(void)
{
  if (_outcnt > 0) {
    // NOTE: we know it's dangerous to cast from (const uint8_t *) to (char *),
    // but we are trying to maintain a stable interface between the Arduino
    // one and the mbed one. What's more, while TCPSocketConnection has no
    // intention of modifying the data here, it requires us to send a (char *)
    // typed data. So we belive it's safe to do the cast here.
    _sock.send_all(const_cast<char*>((const char*) _outbuf), _outcnt);
    _outcnt = 0;
  }
}

void Client::_fillin(void)
{
  int tmp = sizeof(_inbuf) - _incnt;
  if (tmp) {
    tmp = _sock.receive_all((char*)_inbuf + _incnt, tmp);
    if (tmp > 0)
      _incnt += tmp;
  }
}

void Client::flush() {
  _flushout();
}

int Client::available() {
  if (_incnt == 0) {
    _flushout();
    _fillin();
  }
  return (_incnt > 0) ? 1 : 0;
}

int Client::read() {
  uint8_t ch;
  return (read(&ch, 1) == 1) ? ch : -1;
}

int Client::read(uint8_t *buf, size_t size) {
  int cnt = 0;
  while (size) {
    // need more
    if (size > _incnt) {
      _flushout();
      _fillin();
    }
    if (_incnt > 0) {
      int tmp = _incnt;
      if (tmp > size) tmp = size;
      memcpy(buf, _inbuf, tmp);
      if (tmp != _incnt)
          memmove(_inbuf, _inbuf + tmp, _incnt - tmp);
      _incnt -= tmp;
      size -= tmp;
      buf += tmp;
      cnt += tmp;
    } else // no data
        break;
  }
  return cnt;
}

void Client::stop() {
  _sock.close();
}

uint8_t Client::connected() {
  return _sock.is_connected() ? 1 : 0;
}
