#ifndef Client_h
#define Client_h

#include "Print.h"
#include "TCPSocketConnection.h"

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
  uint8_t _buf[1];
  uint8_t _len;
  TCPSocketConnection _sock;
};

#endif
