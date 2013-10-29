#include "M2XStreamClient.h"

#include <jsonlite.h>

#include "StreamParseFunctions.h"
#include "LocationParseFunctions.h"

#ifdef DEBUG
#ifdef ARDUINO_PLATFORM
#define DBG(fmt_, data_) Serial.print(data_)
#define DBGLN(fmt_, data_) Serial.println(data_)
#define DBGLNEND Serial.println()
#endif  // ARDUINO_PLATFORM

#ifdef MBED_PLATFORM
#define DBG(fmt_, data_) printf((fmt_), (data_))
#define DBGLN(fmt_, data_) printf((fmt_), (data_)); printf("\n")
#define DBGLNEND printf("\n")
#endif  // MBED_PLATFORM
#else
#define DBG(fmt_, data_)
#define DBGLN(fmt_, data_)
#define DBGLNEND
#endif  // DEBUG

#define HEX(t_) ((char) (((t_) > 9) ? ((t_) - 10 + 'A') : ((t_) + '0')))
#define MAX_DOUBLE_DIGITS 7

const char* M2XStreamClient::kDefaultM2XHost = "api-m2x.att.com";
const char* kUserAgentLine = "User-Agent: M2X Arduino Client/0.1";

static int print_encoded_string(Print* print, const char* str);

M2XStreamClient::M2XStreamClient(Client* client,
                                 const char* key,
                                 const char* host,
                                 int port) : _client(client),
                                             _key(key),
                                             _host(host),
                                             _port(port),
                                             _null_print() {
}

int M2XStreamClient::send(const char* feedId,
                          const char* streamName,
                          double value) {
  if (_client->connect(_host, _port)) {
    DBGLN("%s", "Connected to M2X server!");
    writeSendHeader(feedId, streamName,
                    // 6 for "value="
                    _null_print.print(value) + 6);
    _client->print("value=");
    // value is a double, does not need encoding
    _client->print(value);
  } else {
    DBGLN("%s", "ERROR: Cannot connect to M2X server!");
    return E_NOCONNECTION;
  }

  return readStatusCode(true);
}

int M2XStreamClient::send(const char* feedId,
                          const char* streamName,
                          long value) {
  if (_client->connect(_host, _port)) {
    DBGLN("%s", "Connected to M2X server!");
    writeSendHeader(feedId, streamName,
                    // 6 for "value="
                    _null_print.print(value) + 6);

    _client->print("value=");
    // value is a long, does not need encoding
    _client->print(value);
  } else {
    DBGLN("%s", "ERROR: Cannot connect to M2X server!");
    return E_NOCONNECTION;
  }

  return readStatusCode(true);
}

int M2XStreamClient::send(const char* feedId,
                          const char* streamName,
                          int value) {
  if (_client->connect(_host, _port)) {
    DBGLN("%s", "Connected to M2X server!");
    writeSendHeader(feedId, streamName,
                    // 6 for "value="
                    _null_print.print(value) + 6);

    _client->print("value=");
    // value is an int, does not need encoding
    _client->print(value);
  } else {
    DBGLN("%s", "ERROR: Cannot connect to M2X server!");
    return E_NOCONNECTION;
  }

  return readStatusCode(true);
}

int M2XStreamClient::send(const char* feedId,
                          const char* streamName,
                          const char* value) {
  if (_client->connect(_host, _port)) {
    DBGLN("%s", "Connected to M2X server!");
    writeSendHeader(feedId, streamName,
                    // 6 for "value="
                    _null_print.print(value) + 6);

    _client->print("value=");
    print_encoded_string(_client, value);
  } else {
    DBGLN("%s", "ERROR: Cannot connect to M2X server!");
    return E_NOCONNECTION;
  }

  return readStatusCode(true);
}

int M2XStreamClient::receive(const char* feedId, const char* streamName,
                             stream_value_read_callback callback, void* context) {
  if (_client->connect(_host, _port)) {
    DBGLN("%s", "Connected to M2X server!");
    _client->print("GET /v1/feeds/");
    print_encoded_string(_client, feedId);
    _client->print("/streams/");
    print_encoded_string(_client, streamName);
    _client->println("/values HTTP/1.0");

    writeHttpHeader(-1);
  } else {
    DBGLN("%s", "ERROR: Cannot connect to M2X server!");
    return E_NOCONNECTION;
  }
  int status = readStatusCode(false);
  if (status == 200) {
    readStreamValue(callback, context);
  }

  close();
  return status;
}

int M2XStreamClient::readLocation(const char* feedId,
                                  location_read_callback callback,
                                  void* context) {
  if (_client->connect(_host, _port)) {
    DBGLN("%s", "Connected to M2X server!");
    _client->print("GET /v1/feeds/");
    print_encoded_string(_client, feedId);
    _client->println("/location HTTP/1.0");

    writeHttpHeader(-1);
  } else {
    DBGLN("%s", "ERROR: Cannot connect to M2X server!");
    return E_NOCONNECTION;
  }
  int status = readStatusCode(false);
  if (status == 200) {
    readLocation(callback, context);
  }

  close();
  return status;
}

// Encodes and prints string using Percent-encoding specified
// in RFC 1738, Section 2.2
static int print_encoded_string(Print* print, const char* str) {
  int bytes = 0;
  for (int i = 0; str[i] != 0; i++) {
    if (((str[i] >= 'A') && (str[i] <= 'Z')) ||
        ((str[i] >= 'a') && (str[i] <= 'z')) ||
        ((str[i] >= '0') && (str[i] <= '9')) ||
        (str[i] == '-') || (str[i] == '_') ||
        (str[i] == '.') || (str[i] == '~')) {
      bytes += print->print(str[i]);
    } else {
      // Encode all other characters
      bytes += print->print('%');
      bytes += print->print(HEX(str[i] / 16));
      bytes += print->print(HEX(str[i] % 16));
    }
  }
  return bytes;
}

static int write_location_data(Print* print, const char* name,
                               double latitude, double longitude,
                               double elevation) {
  int bytes = 0;
  bytes += print->print("name=");
  bytes += print_encoded_string(print, name);
  bytes += print->print("&latitude=");
  bytes += print->print(latitude, MAX_DOUBLE_DIGITS);
  bytes += print->print("&longitude=");
  bytes += print->print(longitude, MAX_DOUBLE_DIGITS);
  bytes += print->print("&elevation=");
  bytes += print->print(elevation);
  return bytes;
}

static int write_location_data(Print* print, const char* name,
                               const char* latitude, const char* longitude,
                               const char* elevation) {
  int bytes = 0;
  bytes += print->print("name=");
  bytes += print_encoded_string(print, name);
  bytes += print->print("&latitude=");
  bytes += print_encoded_string(print, latitude);
  bytes += print->print("&longitude=");
  bytes += print_encoded_string(print, longitude);
  bytes += print->print("&elevation=");
  bytes += print_encoded_string(print, elevation);
  return bytes;
}

int M2XStreamClient::updateLocation(const char* feedId,
                                    const char* name,
                                    double latitude,
                                    double longitude,
                                    double elevation) {
  if (_client->connect(_host, _port)) {
    DBGLN("%s", "Connected to M2X server!");

    int length = write_location_data(&_null_print, name, latitude, longitude,
                                     elevation);
    _client->print("PUT /v1/feeds/");
    print_encoded_string(_client, feedId);
    _client->println("/location HTTP/1.0");

    writeHttpHeader(length);
    write_location_data(_client, name, latitude, longitude, elevation);
  } else {
    DBGLN("%s", "ERROR: Cannot connect to M2X server!");
    return E_NOCONNECTION;
  }
  return readStatusCode(true);
}

int M2XStreamClient::updateLocation(const char* feedId,
                                    const char* name,
                                    const char* latitude,
                                    const char* longitude,
                                    const char* elevation) {
  if (_client->connect(_host, _port)) {
    DBGLN("%s", "Connected to M2X server!");

    int length = write_location_data(&_null_print, name, latitude, longitude,
                                     elevation);
    _client->print("PUT /v1/feeds/");
    print_encoded_string(_client, feedId);
    _client->println("/location HTTP/1.0");

    writeHttpHeader(length);
    write_location_data(_client, name, latitude, longitude, elevation);
  } else {
    DBGLN("%s", "ERROR: Cannot connect to M2X server!");
    return E_NOCONNECTION;
  }
  return readStatusCode(true);
}

void M2XStreamClient::writeSendHeader(const char* feedId,
                                      const char* streamName,
                                      int contentLength) {
  _client->print("PUT /v1/feeds/");
  print_encoded_string(_client, feedId);
  _client->print("/streams/");
  print_encoded_string(_client, streamName);
  _client->println(" HTTP/1.0");

  writeHttpHeader(contentLength);
}

void M2XStreamClient::writeHttpHeader(int contentLength) {
  _client->println(kUserAgentLine);
  _client->print("X-M2X-KEY: ");
  _client->println(_key);

  _client->print("Host: ");
  print_encoded_string(_client, _host);
  if (_port != kDefaultM2XPort) {
    _client->print(":");
    // port is an integer, does not need encoding
    _client->print(_port);
  }
  _client->println();

  if (contentLength > 0) {
    _client->println("Content-Type: application/x-www-form-urlencoded");
    DBG("%s", "Content Length: ");
    DBGLN("%d", contentLength);

    _client->print("Content-Length: ");
    _client->println(contentLength);
  }
  _client->println();
}

int M2XStreamClient::waitForString(const char* str) {
  int currentIndex = 0;
  if (str[currentIndex] == '\0') return E_OK;

  while (true) {
    while (_client->available()) {
      char c = _client->read();
      DBG("%c", c);

      if ((str[currentIndex] == '*') ||
          (c == str[currentIndex])) {
        currentIndex++;
        if (str[currentIndex] == '\0') {
          return E_OK;
        }
      } else {
        // start from the beginning
        currentIndex = 0;
      }
    }

    if (!_client->connected()) {
      DBGLN("%s", "ERROR: The client is disconnected from the server!");

      close();
      return E_DISCONNECTED;
    }

    delay(1000);
  }
  // never reached here
  return E_NOTREACHABLE;
}

int M2XStreamClient::readStatusCode(bool closeClient) {
  int responseCode = 0;
  int ret = waitForString("HTTP/*.* ");
  if (ret != E_OK) {
    if (closeClient) close();
    return ret;
  }

  // ret is not needed from here(since it must be E_OK), so we can use it
  // as a regular variable now.
  ret = 0;
  while (true) {
    while (_client->available()) {
      char c = _client->read();
      DBG("%c", c);

      responseCode = responseCode * 10 + (c - '0');
      ret++;
      if (ret == 3) {
        if (closeClient) close();
        return responseCode;
      }
    }

    if (!_client->connected()) {
      DBGLN("%s", "ERROR: The client is disconnected from the server!");

      if (closeClient) close();
      return E_DISCONNECTED;
    }

    delay(1000);
  }

  // never reached here
  return E_NOTREACHABLE;
}

int M2XStreamClient::readContentLength() {
  int ret = waitForString("Content-Length: ");
  if (ret != E_OK) {
    return ret;
  }

  // From now on, ret is not needed, we can use it
  // to keep the final result
  ret = 0;
  while (true) {
    while (_client->available()) {
      char c = _client->read();
      DBG("%c", c);

      if ((c == '\r') || (c == '\n')) {
        return (ret == 0) ? (E_INVALID) : (ret);
      } else {
        ret = ret * 10 + (c - '0');
      }
    }

    if (!_client->connected()) {
      DBGLN("%s", "ERROR: The client is disconnected from the server!");

      return E_DISCONNECTED;
    }

    delay(1000);
  }

  // never reached here
  return E_NOTREACHABLE;
}

int M2XStreamClient::skipHttpHeader() {
  return waitForString("\r\n\r\n");
}

void M2XStreamClient::close() {
  // Eats up buffered data before closing
  _client->flush();
  _client->stop();
}

int M2XStreamClient::readStreamValue(stream_value_read_callback callback,
                                     void* context) {
  const int BUF_LEN = 32;
  char buf[BUF_LEN];

  int length = readContentLength();
  if (length < 0) {
    close();
    return length;
  }

  int index = skipHttpHeader();
  if (index != E_OK) {
    close();
    return index;
  }
  index = 0;

  stream_parsing_context_state state;
  state.state = state.index = 0;
  state.callback = callback;
  state.context = context;

  jsonlite_parser_callbacks cbs = jsonlite_default_callbacks;
  cbs.key_found = on_stream_key_found;
  cbs.string_found = on_stream_string_found;
  cbs.context.client_state = &state;

  jsonlite_parser p = jsonlite_parser_init(jsonlite_parser_estimate_size(5));
  jsonlite_parser_set_callback(p, &cbs);

  jsonlite_result result = jsonlite_result_unknown;
  while (index < length) {
    int i = 0;

    DBG("%s", "Received Data: ");
    while ((i < BUF_LEN) && _client->available()) {
      buf[i++] = _client->read();
      DBG("%c", buf[i - 1]);
    }
    DBGLNEND;

    if ((!_client->connected()) &&
        (!_client->available()) &&
        ((index + i) < length)) {
      jsonlite_parser_release(p);
      close();
      return E_NOCONNECTION;
    }

    result = jsonlite_parser_tokenize(p, buf, i);
    if ((result != jsonlite_result_ok) &&
        (result != jsonlite_result_end_of_stream)) {
      jsonlite_parser_release(p);
      close();
      return E_JSON_INVALID;
    }

    index += i;
  }

  jsonlite_parser_release(p);
  close();
  return (result == jsonlite_result_ok) ? (E_OK) : (E_JSON_INVALID);
}

int M2XStreamClient::readLocation(location_read_callback callback,
                                  void* context) {
  const int BUF_LEN = 40;
  char buf[BUF_LEN];

  int length = readContentLength();
  if (length < 0) {
    close();
    return length;
  }

  int index = skipHttpHeader();
  if (index != E_OK) {
    close();
    return index;
  }
  index = 0;

  location_parsing_context_state state;
  state.state = state.index = 0;
  state.callback = callback;
  state.context = context;

  jsonlite_parser_callbacks cbs = jsonlite_default_callbacks;
  cbs.key_found = on_location_key_found;
  cbs.string_found = on_location_string_found;
  cbs.context.client_state = &state;

  jsonlite_parser p = jsonlite_parser_init(jsonlite_parser_estimate_size(5));
  jsonlite_parser_set_callback(p, &cbs);

  jsonlite_result result = jsonlite_result_unknown;
  while (index < length) {
    int i = 0;

    DBG("%s", "Received Data: ");
    while ((i < BUF_LEN) && _client->available()) {
      buf[i++] = _client->read();
      DBG("%c", buf[i - 1]);
    }
    DBGLNEND;

    if ((!_client->connected()) &&
        (!_client->available()) &&
        ((index + i) < length)) {
      jsonlite_parser_release(p);
      close();
      return E_NOCONNECTION;
    }

    result = jsonlite_parser_tokenize(p, buf, i);
    if ((result != jsonlite_result_ok) &&
        (result != jsonlite_result_end_of_stream)) {
      jsonlite_parser_release(p);
      close();
      return E_JSON_INVALID;
    }

    index += i;
  }

  jsonlite_parser_release(p);
  close();
  return (result == jsonlite_result_ok) ? (E_OK) : (E_JSON_INVALID);
}
