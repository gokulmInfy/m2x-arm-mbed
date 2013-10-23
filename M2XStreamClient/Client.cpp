#include "Client.h"
#include "mbed.h"

#include <stdint.h>

Client::Client() : _len(0), _sock() {
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
  _sock.set_blocking(false, 15000);
  // NOTE: we know it's dangerous to cast from (const uint8_t *) to (char *),
  // but we are trying to maintain a stable interface between the Arduino
  // one and the mbed one. What's more, while TCPSocketConnection has no
  // intention of modifying the data here, it requires us to send a (char *)
  // typed data. So we belive it's safe to do the cast here.
  return _sock.send_all(const_cast<char*>((const char*) buf), size);
}

int Client::available() {
  if (_len > 0) { return 1; }
  int ret = read(_buf, 1);
  if (ret <= 0) { return 0; }
  _len = ret;
  return 1;
}

int Client::read() {
  if (_len > 0) {
    _len = 0;
    return _buf[0];
  }
  return -1;
}

int Client::read(uint8_t *buf, size_t size) {
  return _sock.receive_all((char*) buf, size);
}

void Client::flush() {
  // does nothing, TCP stack takes care of this
}

void Client::stop() {
  _sock.close();
}

uint8_t Client::connected() {
  return _sock.is_connected();
}
