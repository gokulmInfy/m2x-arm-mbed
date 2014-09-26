#include "Client.h"
#include "mbed.h"

#include <stdint.h>

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
