#ifndef M2XStreamClient_h
#define M2XStreamClient_h

#define MIN(a, b) (((a) > (b))?(b):(a))

#include "mbed.h"
#include "Client.h"
#include "Utility.h"

#include "NullPrint.h"

static const int E_OK = 0;
static const int E_NOCONNECTION = -1;
static const int E_DISCONNECTED = -2;
static const int E_NOTREACHABLE = -3;
static const int E_INVALID = -4;
static const int E_JSON_INVALID = -5;

typedef void (*stream_value_read_callback)(const char* at,
                                           const char* value,
                                           int index,
                                           void* context);

typedef void (*location_read_callback)(const char* name,
                                       double latitude,
                                       double longitude,
                                       double elevation,
                                       const char* timestamp,
                                       int index,
                                       void* context);

class M2XStreamClient {
public:
  static const char* kDefaultM2XHost;
  static const int kDefaultM2XPort = 80;

  M2XStreamClient(Client* client,
                  const char* key,
                  const char* host = kDefaultM2XHost,
                  int port = kDefaultM2XPort);

  // Update data stream, returns the HTTP status code
  int send(const char* feedId, const char* streamName, double value);
  int send(const char* feedId, const char* streamName, long value);
  int send(const char* feedId, const char* streamName, int value);
  int send(const char* feedId, const char* streamName, const char* value);

  // Receive values for a particular data stream. Since memory is
  // very limited on an Arduino, we cannot parse and get all the
  // data points in memory. Instead, we use callbacks here: whenever
  // a new data point is parsed, we call the callback using the values,
  // after that, the values will be thrown away to make space for new
  // values.
  // Note that you can also pass in a user-specified context in this
  // function, this context will be passed to the callback function
  // each time we get a data point.
  // For each data point, the callback will be called once. The HTTP
  // status code will be returned. And the content is only parsed when
  // the status code is 200.
  int receive(const char* feedId, const char* streamName,
              stream_value_read_callback callback, void* context);

  // Update datasource location
  // NOTE: On an Arduino Uno and other ATMEGA based boards, double has
  // 4-byte (32 bits) precision, which is the same as float. So there's
  // no natural double-precision floating number on these boards. With
  // a float value, we have a precision of roughly 7 digits, that means
  // either 5 or 6 digits after the floating point. According to wikipedia,
  // a difference of 0.00001 will give us ~1.1132m distance. If this
  // precision is good for you, you can use the double-version we provided
  // here. Otherwise, you may need to use the string-version and do the
  // actual conversion by yourselves.
  // However, with an Arduino Due board, double has 8-bytes (64 bits)
  // precision, which means you are free to use the double-version only
  // without any precision problems.
  // Returned value is the http status code.
  int updateLocation(const char* feedId, const char* name,
                     double latitude, double longitude, double elevation);
  int updateLocation(const char* feedId, const char* name,
                     const char* latitude, const char* longitude,
                     const char* elevation);

  // Read location information for a feed. Also used callback to process
  // data points for memory reasons. The HTTP status code is returned,
  // response is only parsed when the HTTP status code is 200
  int readLocation(const char* feedId, location_read_callback callback,
                   void* context);
private:
  Client* _client;
  const char* _key;
  const char* _host;
  int _port;
  NullPrint _null_print;

  // Writes the HTTP header part for updating a stream value
  void writeSendHeader(const char* feedId,
                       const char* streamName,
                       int contentLength);
  // Writes HTTP header lines including M2X API Key, host, content
  // type and content length(if the body exists)
  void writeHttpHeader(int contentLength);
  // Parses HTTP response header and return the content length.
  // Note that this function does not parse all http headers, as long
  // as the content length is found, this function will return
  int readContentLength();
  // Skips all HTTP response header part. Return minus value in case
  // the connection is closed before we got all headers
  int skipHttpHeader();
  // Parses and returns the HTTP status code, note this function will
  // return immediately once it gets the status code
  int readStatusCode(bool closeClient);
  // Waits for a certain string pattern in the HTTP header, and returns
  // once the pattern is found. In the pattern, you can use '*' to denote
  // any character
  int waitForString(const char* str);
  // Closes the connection
  void close();
  // Parses JSON response of stream value API, and calls callback function
  // once we get a data point
  int readStreamValue(stream_value_read_callback callback, void* context);
  // Parses JSON response of location API, and calls callback function once
  // we get a data point
  int readLocation(location_read_callback callback, void* context);
};

#endif  /* M2XStreamClient_h */

