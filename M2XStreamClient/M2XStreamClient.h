#ifndef M2XStreamClient_h
#define M2XStreamClient_h

#if (!defined(ARDUINO_PLATFORM)) && (!defined(ESP8266_PLATFORM)) && (!defined(MBED_PLATFORM))
#error "Platform definition is missing!"
#endif

#define M2X_VERSION "2.2.0"

#ifdef ARDUINO_PLATFORM
#include "m2x-arduino.h"
#endif  /* ARDUINO_PLATFORM */

#ifdef ESP8266_PLATFORM
#include "m2x-esp8266.h"
#endif /* ESP8266_PLATFORM */

#ifdef MBED_PLATFORM
#include "m2x-mbed.h"
#endif /* MBED_PLATFORM */

/* If we don't have DBG defined, provide dump implementation */
#ifndef DBG
#define DBG(fmt_, data_)
#define DBGLN(fmt_, data_)
#define DBGLNEND
#endif  /* DBG */

#define MIN(a, b) (((a) > (b))?(b):(a))
#define TO_HEX(t_) ((char) (((t_) > 9) ? ((t_) - 10 + 'A') : ((t_) + '0')))
#define MAX_DOUBLE_DIGITS 7

/* For tolower */
#include <ctype.h>

static const int E_OK = 0;
static const int E_NOCONNECTION = -1;
static const int E_DISCONNECTED = -2;
static const int E_NOTREACHABLE = -3;
static const int E_INVALID = -4;
static const int E_JSON_INVALID = -5;
static const int E_BUFFER_TOO_SMALL = -6;
static const int E_TIMESTAMP_ERROR = -8;

static const char* DEFAULT_M2X_HOST = "api-m2x.att.com";
static const int DEFAULT_M2X_PORT = 80;

static inline bool m2x_status_is_success(int status) {
  return (status == E_OK) || (status >= 200 && status <= 299);
}

static inline bool m2x_status_is_client_error(int status) {
  return status >= 400 && status <= 499;
}

static inline bool m2x_status_is_server_error(int status) {
  return status >= 500 && status <= 599;
}

static inline bool m2x_status_is_error(int status) {
  return m2x_status_is_client_error(status) ||
      m2x_status_is_server_error(status);
}
/*
 * Null Print class used to calculate length to print
 */
class NullPrint : public Print {
public:
  size_t counter;

  virtual size_t write(uint8_t b) {
    counter++;
    return 1;
  }

  virtual size_t write(const uint8_t* buf, size_t size) {
    counter += size;
    return size;
  }
};

// Encodes and prints string using Percent-encoding specified
// in RFC 1738, Section 2.2
static inline int print_encoded_string(Print* print, const char* str) {
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
      bytes += print->print(TO_HEX(str[i] / 16));
      bytes += print->print(TO_HEX(str[i] % 16));
    }
  }
  return bytes;
}

#ifdef M2X_ENABLE_READER
/**
 * +type+ indicates the value type: 1 for string, 2 for number
 * NOTE that the value type here only contains a hint on how
 * you can use the value. Even though 2 is returned, the value
 * is still stored in (const char *), and atoi/atof is needed to
 * get the actual value
 */
typedef void (*stream_value_read_callback)(const char* at,
                                           const char* value,
                                           int index,
                                           void* context,
                                           int type);

typedef void (*location_read_callback)(const char* name,
                                       double latitude,
                                       double longitude,
                                       double elevation,
                                       const char* timestamp,
                                       int index,
                                       void* context);

typedef void (*m2x_command_read_callback)(const char* id,
                                          const char* name,
                                          int index,
                                          void *context);
#endif  /* M2X_ENABLE_READER */

typedef void (*m2x_fill_data_callback)(Print *print, void *context);

class M2XStreamClient {
public:
  M2XStreamClient(Client* client,
                  const char* key,
                  void (* idlefunc)(void) = NULL,
                  int case_insensitive = 1,
                  const char* host = DEFAULT_M2X_HOST,
                  int port = DEFAULT_M2X_PORT,
                  const char* path_prefix = NULL);
/**
 * Method for <a href="https://m2x.att.com/developer/documentation/v2/device#Update-Data-Stream-Value">Update Data Stream Value</a> endpoint.
 * Push data stream value using PUT request, returns the HTTP status code
 * @param deviceId - id of the device to post values
 * @param streamName - Name of the stream.
 * @param value - the value.<p>
 * NOTE: if you want to update by a serial, use "serial/<serial ID>" as the device ID here.
 */
 
  template <class T>
  int updateStreamValue(const char* deviceId, const char* streamName, T value);

  /**
   * Method for <a href="https://m2x.att.com/developer/documentation/v2/device#Post-Device-Updates--Multiple-Values-to-Multiple-Streams-">Post Device Updates (Multiple Values to Multiple Streams)</a> endpoint.
   * Post multiple values to M2X all at once.
   * @param deviceId - id of the device to post values
   * @param streamNum - Number of streams to post
   * @param names - Array of stream names, the length of the array should be exactly +streamNum+
   * @param counts - Array of +streamNum+ length, each item in this array containing the number of values we want to post for each stream
   * @param ats - Timestamps for each value, the length of this array should be the some of all values in +counts+, for the first +counts[0]+
   * items, the values belong to the first stream, for the following
   * +counts[1]+ number of items, the values belong to the second stream,
   * etc. Notice that timestamps are required here: you must provide
   * a timestamp for each value posted.
   * @param values - Values to post. This works the same way as +ats+, the
   * first +counts[0]+ number of items contain values to post to the first
   * stream, the succeeding +counts[1]+ number of items contain values
   * for the second stream, etc. The length of this array should be
   * the sum of all values in +counts+ array.<p>
   * NOTE: if you want to update by a serial, use "serial/<serial ID>" as
   * the device ID here.
   */
  template <class T>
  int postDeviceUpdates(const char* deviceId, int streamNum,
                        const char* names[], const int counts[],
                        const char* ats[], T values[]);

 /**
  * Method for <a href="https://m2x.att.com/developer/documentation/v2/device#Post-Device-Update--Single-Values-to-Multiple-Streams-">Post Device Update</a> endpoint.
  * Post multiple values of a single device at once.
  * @param deviceId - id of the device to post values
  * @param streamNum - Number of streams to post
  * @param names - Array of stream names, the length of the array should  be exactly +streamNum+
  * @param values - Array of values to post, the length of the array should  be exactly +streamNum+. Notice that the array of +values+ should
  * match the array of +names+, and that the ith value in +values+ is  exactly the value to post for the ith stream name in +names+<p>
  * NOTE: if you want to update by a serial, use "serial/<serial ID>" as  the device ID here.
  */
  template <class T>
  int postDeviceUpdate(const char* deviceId, int streamNum,
                       const char* names[], T values[],
                       const char* at = NULL);

#ifdef M2X_ENABLE_READER
  // Fetch values for a particular data stream. Since memory is
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
  int listStreamValues(const char* deviceId, const char* streamName,
                       stream_value_read_callback callback, void* context,
                       const char* query = NULL);
#endif  /* M2X_ENABLE_READER */

 /**
  * Method for <a href="https://m2x.att.com/developer/documentation/v2/device#Update-Device-Location">Update Device Location</a> endpoint.
  * Update datasource location
  * @param deviceId - id of the device.
  * @param name - a name identifying this location
  * @param latitude - latitude
  * @param longitude - longitude
  * @param elevation - elevation
  * NOTE: On an Arduino Uno and other ATMEGA based boards, double has  4-byte (32 bits) precision,
  * which is the same as float. So there's  no natural double-precision floating number on these boards.
  * With  a float value, we have a precision of roughly 7 digits, that means  either 5 or 6 digits after the floating point.
  * According to wikipedia,  a difference of 0.00001 will give us ~1.1132m distance.
  * If this  precision is good for you, you can use the double-version we provided  here.
  * Otherwise, you may need to use the string-version and do the  actual conversion by yourselves.
  * However, with an Arduino Due board, double has 8-bytes (64 bits)  precision,
  * which means you are free to use the double-version only without any precision problems.
  * Returned value is the http status code.<p>
  * NOTE: if you want to update by a serial, use "serial/<serial ID>" as  the device ID here.
  */
  
  template <class T>
  int updateLocation(const char* deviceId, const char* name,
                     T latitude, T longitude, T elevation);

#ifdef M2X_ENABLE_READER
  // Read location information for a device. Also used callback to process
  // data points for memory reasons. The HTTP status code is returned,
  // response is only parsed when the HTTP status code is 200
  int readLocation(const char* deviceId, location_read_callback callback,
                   void* context);
#endif  /* M2X_ENABLE_READER */

/**
 * Method for <a href="https://m2x.att.com/developer/documentation/v2/device#Delete-Data-Stream-Values">Delete Data Stream Values</a> endpoint.
 * Delete values from a data stream
 * @param deviceId - id of the device to post values<p>
 * You will need to provide from and end date/time strings in the ISO8601
 * format "yyyy-mm-ddTHH:MM:SS.SSSZ" where
 *     yyyy: the year
 *     mm: the month
 *     dd: the day
 *     HH: the hour (24 hour format)
 *     MM: the minute
 *     SS.SSS: the seconds (to the millisecond)<p>
 * NOTE: the time is given in Zulu (GMT) M2X will delete all values within the from to end date/time range.
 * The status code is 204 on success and 400 on a bad request (e.g. the timestamp is not in ISO8601 format or the from timestamp is not less than
 * or equal to the end timestamp.
 */
  int deleteValues(const char* deviceId, const char* streamName,
                   const char* from, const char* end);

#ifdef M2X_ENABLE_READER
  // Fetch commands available for this device, notice that for memory constraints,
  // we only keep ID and name of the command received here.
  // You can tweak the command receiving via the query parameter.
  int listCommands(const char* deviceId,
                   m2x_command_read_callback callback, void* context,
                   const char* query = NULL);
#endif  /* M2X_ENABLE_READER */

 /**
  * Method for <a href="https://m2x.att.com/developer/documentation/v2/commands#Device-Marks-a-Command-as-Processed">Device Marks a Command as Processed</a> endpoint.
  * @param deviceId - id of the device
  * @param commandId - id of the command
  * @param callback - To make sure the minimal amount of memory is needed, this API works with
  * a callback function. The callback function is then used to fill the request
  * data, note that in order to correctly set the content length in HTTP header,
  * the callback function will be called twice, the caller must make sure both
  * calls fill the Print object with exactly the same data.
  * If you have a pre-allocated buffer filled with the data to post, you can
  * use markCommandProcessedWithData API below.
  *
  * @param context - the context
  * */
  int markCommandProcessed(const char* deviceId, const char* commandId,
                           m2x_fill_data_callback callback, void *context);
/**
 * Method for <a href="https://m2x.att.com/developer/documentation/v2/commands#Device-Marks-a-Command-as-Processed">Device Marks a Command as Processed</a> endpoint.
 * Mark a command as processed with a data buffer.
 * This is exactly like markCommandProcessed, except that a buffer is use to
 * contain the data to post as the request body.
 * @param deviceId - id of the device
 * @param commandId - id of the command
 * @param data - the data buffer
 */
  int markCommandProcessedWithData(const char* deviceId, const char* commandId,
                                   const char* data);
/**
 * Method for <a href="https://m2x.att.com/developer/documentation/v2/commands#Device-Marks-a-Command-as-Rejected">Device Marks a Command as Rejected</a> endpoint.
 * Mark a command as rejected.
 * @param deviceId - id of the device
 * @param commandId - id of the command
 * @param callback - To make sure the minimal amount of memory is needed, this API works with
 * a callback function. The callback function is then used to fill the request data, note that in order to correctly set the content length in HTTP header,
 * the callback function will be called twice, the caller must make sure both calls fill the Print object with exactly the same data.
 * If you have a pre-allocated buffer filled with the data to post, you can use markCommandRejectedWithData API below.
 */
  int markCommandRejected(const char* deviceId, const char* commandId,
                          m2x_fill_data_callback callback, void *context);
/**
 * Method for <a href="https://m2x.att.com/developer/documentation/v2/commands#Device-Marks-a-Command-as-Rejected">Device Marks a Command as Rejected</a> endpoint.
 * Mark a command as rejected with a data buffer
 * This is exactly like markCommandRejected, except that a buffer is use to contain the data to post as the request body.
 * @param deviceId - id of the device
 * @param commandId - id of the command
 * @param data - the data buffer
 */
  int markCommandRejectedWithData(const char* deviceId, const char* commandId,
                                  const char* data);

  /**
   * Fetches current timestamp in seconds from M2X server. Since we
   * are using signed 32-bit integer as return value, this will only
   * return valid results before 03:14:07 UTC on 19 January 2038. If
   * the device is supposed to work after that, this function should
   * not be used.<p>
   * The returned value will contain the status code(positive values)
   * or the error code(negative values).In case of success, the current timestamp will be filled in the
   *  +ts+ pointer passed in as argument.<p>
   *  NOTE: although returning uint32_t can give us a larger space,
   *   we prefer to cope with the unix convention here.
   */
  int getTimestamp32(int32_t* ts);

  /**
   * Fetches current timestamp in seconds from M2X server.
   * This function will return the timestamp as an integer literal
   * in the provided buffer. Hence there's no problem working after
   * 03:14:07 UTC on 19 January 2038. The drawback part here, is that
   * you will have to work with 64-bit integer, which is not available
   * on certain platform(such as Arduino), a bignum library or alike
   * is needed in this case.<p>
   * Notice +bufferLength+ is supposed to contain the length of the
   * buffer when calling this function. It is also the caller's
   * responsibility to ensure the buffer is big enough, otherwise
   * the library will return an error indicating the buffer is too small.
   * While this is not accurate all the time, one trick here is to
   * pass in 0 as the bufferLength, in which case we will always return
   * the buffer-too-small error. However, the correct buffer length
   * can be found this way so a secound execution is most likely to work
   * (unless we are at the edge of the buffer length increasing, for
   * example, when the timestamp jumps from 9999999999 to 10000000000,
   * which is highly unlikely to happend). However, given that the
   * maximum 64-bit integer can be stored in 19 bytes, there's not
   * much need to use this trick.)<p>
   * The returned value will contain the status code(positive values)
   * or the error code(negative values).<p>
   * In case of success, the current timestamp will be filled in the
   * passed +buffer+ pointer, and the actual used buffer length will
   * be returned in +bufferLength+ pointer.<p>
   * NOTE: as long as we can read the returned buffer length, it will
   * be used to fill in the +bufferLength+ variable even though other
   * errors occur(buffer is not enough, network is shutdown before
   * reading the whole buffer, etc.)
   */
  int getTimestamp(char* buffer, int* bufferLength);
private:
  Client* _client;
  const char* _key;
  int _case_insensitive;
  const char* _host;
  int _port;
  void (* _idlefunc)(void);
  const char* _path_prefix;
  NullPrint _null_print;

   /**
    * Writes the HTTP header part for updating a stream value
    */
  void writePutHeader(const char* deviceId,
                      const char* streamName,
                      int contentLength);
  /**
   *  Writes the HTTP header part for deleting stream values
   */
  void writeDeleteHeader(const char* deviceId,
                         const char* streamName,
                         int contentLength);
  /**
   * Writes HTTP header lines including M2X API Key, host, content
   * type and content length(if the body exists)
   */
  void writeHttpHeader(int contentLength);

  /**
   * Parses HTTP response header and return the content length.
   * Note that this function does not parse all http headers, as long
   * as the content length is found, this function will return
   */
  int readContentLength();

   /**
    * Skips all HTTP response header part. Return minus value in case
    * the connection is closed before we got all headers
    */
  int skipHttpHeader();

  /**
   * Parses and returns the HTTP status code, note this function will
   * return immediately once it gets the status code
   */
  int readStatusCode(bool closeClient);

  /**
   * Waits for a certain string pattern in the HTTP header, and returns
   * once the pattern is found. In the pattern, you can use '*' to denote
   * any character
   */
  int waitForString(const char* str);

  /** Closes the connection
   */
  void close();

#ifdef M2X_ENABLE_READER
  // Parses JSON response of stream value API, and calls callback function
  // once we get a data point
  int readStreamValue(stream_value_read_callback callback, void* context);
  // Parses JSON response of location API, and calls callback function once
  // we get a data point
  int readLocation(location_read_callback callback, void* context);
  // Parses JSON response of command API, and calls callback function once
  // we get a data point
  int readCommand(m2x_command_read_callback callback, void* context);
#endif  /* M2X_ENABLE_READER */
};


// A ISO8601 timestamp generation service for M2X.
// It uses the Time API provided by the M2X server to initialize
// clock, then uses millis() function provided by Arduino to calculate
// time advancements so as to reduce API query times.
//
// Right now, this service only works with 32-bit timestamp, meaning that
// this service won't work after 03:14:07 UTC on 19 January 2038. However,
// a similar service that uses 64-bit timestamp can be implemented following
// the logic here.
class TimeService {
public:
  TimeService(M2XStreamClient* client);

  // Initialize the time service. Notice the TimeService instance is only
  // working after calling this function successfully.
  int init();

  // Reset the internal recorded time by calling M2X Time API again. Normally,
  // you don't need to call this manually. TimeService will handle Arduino clock
  // overflow automatically
  int reset();

  // Fills ISO8601 formatted timestamp into the buffer provided. +length+ should
  // contains the maximum supported length of the buffer when calling. For now,
  // the buffer should be able to store 25 characters for a full ISO8601 formatted
  // timestamp, otherwise, an error will be returned.
  int getTimestamp(char* buffer, int* length);
private:
  M2XStreamClient* _client;
  int32_t _server_timestamp;
  uint32_t _local_last_milli;
  M2XTimer _timer;
};


// Implementations
M2XStreamClient::M2XStreamClient(Client* client,
                                 const char* key,
                                 void (* idlefunc)(void),
                                 int case_insensitive,
                                 const char* host,
                                 int port,
                                 const char* path_prefix) : _client(client),
                                             _key(key),
                                             _idlefunc(idlefunc),
                                             _case_insensitive(case_insensitive),
                                             _host(host),
                                             _port(port),
                                             _path_prefix(path_prefix),
                                             _null_print() {
}

template <class T>
int M2XStreamClient::updateStreamValue(const char* deviceId, const char* streamName, T value) {
  if (_client->connect(_host, _port)) {
    DBGLN("%s", "Connected to M2X server!");
    writePutHeader(deviceId, streamName,
                   //  for {"value": and }
                   _null_print.print(value) + 12);
    _client->print("{\"value\":\"");
    _client->print(value);
    _client->print("\"}");
  } else {
    DBGLN("%s", "ERROR: Cannot connect to M2X server!");
    return E_NOCONNECTION;
  }

  return readStatusCode(true);
}

template <class T>
inline int write_multiple_values(Print* print, int streamNum,
                                 const char* names[], const int counts[],
                                 const char* ats[], T values[]) {
  int bytes = 0, value_index = 0;
  bytes += print->print("{\"values\":{");
  for (int i = 0; i < streamNum; i++) {
    bytes += print->print("\"");
    bytes += print->print(names[i]);
    bytes += print->print("\":[");
    for (int j = 0; j < counts[i]; j++) {
      bytes += print->print("{\"timestamp\": \"");
      bytes += print->print(ats[value_index]);
      bytes += print->print("\",\"value\": \"");
      bytes += print->print(values[value_index]);
      bytes += print->print("\"}");
      if (j < counts[i] - 1) { bytes += print->print(","); }
      value_index++;
    }
    bytes += print->print("]");
    if (i < streamNum - 1) { bytes += print->print(","); }
  }
  bytes += print->print("}}");
  return bytes;
}

template <class T>
int M2XStreamClient::postDeviceUpdates(const char* deviceId, int streamNum,
                                       const char* names[], const int counts[],
                                       const char* ats[], T values[]) {
  if (_client->connect(_host, _port)) {
    DBGLN("%s", "Connected to M2X server!");
    int length = write_multiple_values(&_null_print, streamNum, names,
                                       counts, ats, values);
    _client->print("POST ");
    if (_path_prefix) { _client->print(_path_prefix); }
    _client->print("/v2/devices/");
    _client->print(deviceId);
    _client->println("/updates HTTP/1.0");
    writeHttpHeader(length);
    write_multiple_values(_client, streamNum, names, counts, ats, values);
  } else {
    DBGLN("%s", "ERROR: Cannot connect to M2X server!");
    return E_NOCONNECTION;
  }
  return readStatusCode(true);
}

template <class T>
inline int write_single_device_values(Print* print, int streamNum,
                                      const char* names[], T values[],
                                      const char* at) {
  int bytes = 0;
  bytes += print->print("{\"values\":{");
  for (int i = 0; i < streamNum; i++) {
    bytes += print->print("\"");
    bytes += print->print(names[i]);
    bytes += print->print("\": \"");
    bytes += print->print(values[i]);
    bytes += print->print("\"");
    if (i < streamNum - 1) { bytes += print->print(","); }
  }
  bytes += print->print("}");
  if (at != NULL) {
    bytes += print->print(",\"timestamp\":\"");
    bytes += print->print(at);
    bytes += print->print("\"");
  }
  bytes += print->print("}");
  return bytes;
}

template <class T>
int M2XStreamClient::postDeviceUpdate(const char* deviceId, int streamNum,
                                      const char* names[], T values[],
                                      const char* at) {
  if (_client->connect(_host, _port)) {
    DBGLN("%s", "Connected to M2X server!");
    int length = write_single_device_values(&_null_print, streamNum, names,
                                            values, at);
    _client->print("POST ");
    if (_path_prefix) { _client->print(_path_prefix); }
    _client->print("/v2/devices/");
    _client->print(deviceId);
    _client->println("/update HTTP/1.0");
    writeHttpHeader(length);
    write_single_device_values(_client, streamNum, names, values, at);
  } else {
    DBGLN("%s", "ERROR: Cannot connect to M2X server!");
    return E_NOCONNECTION;
  }
  return readStatusCode(true);
}

template <class T>
static int write_location_data(Print* print, const char* name,
                               T latitude, T longitude,
                               T elevation) {
  int bytes = 0;
  bytes += print->print("{\"name\":\"");
  bytes += print->print(name);
  bytes += print->print("\",\"latitude\":\"");
  bytes += print->print(latitude);
  bytes += print->print("\",\"longitude\":\"");
  bytes += print->print(longitude);
  bytes += print->print("\",\"elevation\":\"");
  bytes += print->print(elevation);
  bytes += print->print("\"}");
  return bytes;
}

static int write_location_data(Print* print, const char* name,
                               double latitude, double longitude,
                               double elevation) {
  int bytes = 0;
  bytes += print->print("{\"name\":\"");
  bytes += print->print(name);
  bytes += print->print("\",\"latitude\":\"");
  bytes += print->print(latitude, MAX_DOUBLE_DIGITS);
  bytes += print->print("\",\"longitude\":\"");
  bytes += print->print(longitude, MAX_DOUBLE_DIGITS);
  bytes += print->print("\",\"elevation\":\"");
  bytes += print->print(elevation);
  bytes += print->print("\"}");
  return bytes;
}

template <class T>
int M2XStreamClient::updateLocation(const char* deviceId,
                                    const char* name,
                                    T latitude,
                                    T longitude,
                                    T elevation) {
  if (_client->connect(_host, _port)) {
    DBGLN("%s", "Connected to M2X server!");

    int length = write_location_data(&_null_print, name, latitude, longitude,
                                     elevation);
    _client->print("PUT ");
    if (_path_prefix) { _client->print(_path_prefix); }
    _client->print("/v2/devices/");
    _client->print(deviceId);
    _client->println("/location HTTP/1.0");

    writeHttpHeader(length);
    write_location_data(_client, name, latitude, longitude, elevation);
  } else {
    DBGLN("%s", "ERROR: Cannot connect to M2X server!");
    return E_NOCONNECTION;
  }
  return readStatusCode(true);
}

static inline int write_delete_values(Print* print, const char* from,
                                      const char* end) {
  int bytes = 0;
  bytes += print->print("{\"from\":\"");
  bytes += print->print(from);
  bytes += print->print("\",\"end\":\"");
  bytes += print->print(end);
  bytes += print->print("\"}");
  return bytes;
}

int M2XStreamClient::deleteValues(const char* deviceId, const char* streamName,
                                  const char* from, const char* end) {
  if (_client->connect(_host, _port)) {
    DBGLN("%s", "Connected to M2X server!");
    int length = write_delete_values(&_null_print, from, end);
    writeDeleteHeader(deviceId, streamName, length);
    write_delete_values(_client, from, end);
  } else {
    DBGLN("%s", "ERROR: Cannot connect to M2X server!");
    return E_NOCONNECTION;
  }

  return readStatusCode(true);
}

int M2XStreamClient::markCommandProcessed(const char* deviceId,
                                          const char* commandId,
                                          m2x_fill_data_callback callback,
                                          void *context) {
  if (_client->connect(_host, _port)) {
    DBGLN("%s", "Connected to M2X server!");
    _null_print.counter = 0;
    callback(&_null_print, context);
    int length = _null_print.counter;
    _client->print("POST ");
    if (_path_prefix) { _client->print(_path_prefix); }
    _client->print("/v2/devices/");
    _client->print(deviceId);
    _client->print("/commands/");
    _client->print(commandId);
    _client->print("/process");
    _client->println(" HTTP/1.0");
    writeHttpHeader(length);
    callback(_client, context);
  } else {
    DBGLN("%s", "ERROR: Cannot connect to M2X server!");
    return E_NOCONNECTION;
  }
  return readStatusCode(true);
}

static void m2x_fixed_buffer_filling_callback(Print *print, void *context) {
  if (context) {
    print->print((const char *) context);
  }
}

int M2XStreamClient::markCommandProcessedWithData(const char* deviceId,
                                                  const char* commandId,
                                                  const char* data) {
  return markCommandProcessed(deviceId, commandId,
                              m2x_fixed_buffer_filling_callback, (void *) data);
}

int M2XStreamClient::markCommandRejected(const char* deviceId,
                                         const char* commandId,
                                         m2x_fill_data_callback callback,
                                         void *context) {
  if (_client->connect(_host, _port)) {
    DBGLN("%s", "Connected to M2X server!");
    _null_print.counter = 0;
    callback(&_null_print, context);
    int length = _null_print.counter;
    _client->print("POST ");
    if (_path_prefix) { _client->print(_path_prefix); }
    _client->print("/v2/devices/");
    _client->print(deviceId);
    _client->print("/commands/");
    _client->print(commandId);
    _client->print("/reject");
    _client->println(" HTTP/1.0");
    writeHttpHeader(length);
    callback(_client, context);
  } else {
    DBGLN("%s", "ERROR: Cannot connect to M2X server!");
    return E_NOCONNECTION;
  }
  return readStatusCode(true);
}

int M2XStreamClient::markCommandRejectedWithData(const char* deviceId,
                                                 const char* commandId,
                                                 const char* data) {
  return markCommandRejected(deviceId, commandId,
                             m2x_fixed_buffer_filling_callback, (void *) data);
}

int M2XStreamClient::getTimestamp32(int32_t *ts) {
  // The maximum value of signed 64-bit integer is 0x7fffffffffffffff,
  // which is 9223372036854775807. It consists of 19 characters, so a
  // buffer of 20 is definitely enough here
  int length = 20;
  char buffer[20];
  int status = getTimestamp(buffer, &length);
  if (status == 200) {
    int32_t result = 0;
    for (int i = 0; i < length; i++) {
      result = result * 10 + (buffer[i] - '0');
    }
    if (ts != NULL) { *ts = result; }
  }
  return status;
}

int M2XStreamClient::getTimestamp(char* buffer, int *bufferLength) {
  if (bufferLength == NULL) { return E_INVALID; }
  if (_client->connect(_host, _port)) {
    DBGLN("%s", "Connected to M2X server!");
    _client->print("GET ");
    if (_path_prefix) { _client->print(_path_prefix); }
    _client->println("/v2/time/seconds HTTP/1.0");

    writeHttpHeader(-1);
  } else {
    DBGLN("%s", "ERROR: Cannot connect to M2X server!");
    return E_NOCONNECTION;
  }
  int status = readStatusCode(false);
  if (status == 200) {
    int length = readContentLength();
    if (length < 0) {
      close();
      return length;
    }
    if (*bufferLength < length) {
      *bufferLength = length;
      return E_BUFFER_TOO_SMALL;
    }
    *bufferLength = length;
    int index = skipHttpHeader();
    if (index != E_OK) {
      close();
      return index;
    }
    index = 0;
    while (index < length) {
      DBG("%s", "Received Data: ");
      while ((index < length) && _client->available()) {
        buffer[index++] = _client->read();
        DBG("%c", buffer[index - 1]);
      }
      DBGLNEND;

      if ((!_client->connected()) &&
          (index < length)) {
        close();
        return E_NOCONNECTION;
      }

      if (_idlefunc!=NULL) {
        _idlefunc();
      } else {
        delay(200);
      }
    }
  }
  close();
  return status;
}


void M2XStreamClient::writePutHeader(const char* deviceId,
                                     const char* streamName,
                                     int contentLength) {
  _client->print("PUT ");
  if (_path_prefix) { _client->print(_path_prefix); }
  _client->print("/v2/devices/");
  _client->print(deviceId);
  _client->print("/streams/");
  print_encoded_string(_client, streamName);
  _client->println("/value HTTP/1.0");

  writeHttpHeader(contentLength);
}

void M2XStreamClient::writeDeleteHeader(const char* deviceId,
                                        const char* streamName,
                                        int contentLength) {
  _client->print("DELETE ");
  if (_path_prefix) { _client->print(_path_prefix); }
  _client->print("/v2/devices/");
  _client->print(deviceId);
  _client->print("/streams/");
  print_encoded_string(_client, streamName);
  _client->print("/values");
  _client->println(" HTTP/1.0");

  writeHttpHeader(contentLength);
}

void M2XStreamClient::writeHttpHeader(int contentLength) {
  _client->println(USER_AGENT);
  _client->print("X-M2X-KEY: ");
  _client->println(_key);

  _client->print("Host: ");
  print_encoded_string(_client, _host);
  if (_port != DEFAULT_M2X_PORT) {
    _client->print(":");
    // port is an integer, does not need encoding
    _client->print(_port);
  }
  _client->println();

  if (contentLength > 0) {
    _client->println("Content-Type: application/json");
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

      int cmp;
      if (_case_insensitive) {
        cmp = tolower(c) - tolower(str[currentIndex]);
      } else {
        cmp = c - str[currentIndex];
      }

      if ((str[currentIndex] == '*') || (cmp == 0)) {
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
    if (_idlefunc!=NULL)
      _idlefunc();
    else
      delay(200);
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
    if (_idlefunc!=NULL)
      _idlefunc();
    else
      delay(200);
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
    if (_idlefunc!=NULL)
      _idlefunc();
    else
      delay(200);
  }

  // never reached here
  return E_NOTREACHABLE;
}

int M2XStreamClient::skipHttpHeader() {
  return waitForString("\n\r\n");
}

void M2XStreamClient::close() {
  // Eats up buffered data before closing
  _client->flush();
  _client->stop();
}

static inline int fill_iso8601_timestamp(int32_t seconds, int32_t milli,
                                         char* buffer, int* length);

TimeService::TimeService(M2XStreamClient* client) : _client(client) {
}

int TimeService::init() {
  _timer.start();
  return reset();
}

int TimeService::reset() {
  int32_t ts;
  int status = _client->getTimestamp32(&ts);

  if (m2x_status_is_success(status)) {
    _server_timestamp = ts;
    _local_last_milli = _timer.read_ms();
  }

  return status;
}

int TimeService::getTimestamp(char* buffer, int* length) {
  uint32_t now = _timer.read_ms();
  if (now < _local_last_milli) {
    // In case of a timestamp overflow(happens once every 50 days on
    // Arduino), we reset the server timestamp recorded.
    // NOTE: while on an Arduino this might be okay, the situation is worse
    // on mbed, see the notes in m2x-mbed.h for details
    int status = reset();
    if (!m2x_status_is_success(status)) { return status; }
    now = _timer.read_ms();
  }
  if (now < _local_last_milli) {
    // We have already reseted the timestamp, so this cannot happen
    // (an HTTP request can take longer than 50 days to finished? You
    // must be kidding here). Something else must be wrong here
    return E_TIMESTAMP_ERROR;
  }
  uint32_t diff = now - _local_last_milli;
  _local_last_milli = now;
  _server_timestamp += (int32_t) (diff / 1000); // Milliseconds to seconds
  return fill_iso8601_timestamp(_server_timestamp, (int32_t) (diff % 1000),
                                buffer, length);
}

#define SIZE_ISO_8601 25
static inline bool is_leap_year(int16_t y) {
  return ((1970 + y) > 0) &&
      !((1970 + y) % 4) &&
      (((1970 + y) % 100) || !((1970 + y) % 400));
}
static inline int32_t days_in_year(int16_t y) {
  return is_leap_year(y) ? 366 : 365;
}
static const uint8_t MONTH_DAYS[]={31,28,31,30,31,30,31,31,30,31,30,31};

static inline int fill_iso8601_timestamp(int32_t timestamp, int32_t milli,
                                         char* buffer, int* length) {
  int16_t year;
  int8_t month, month_length;
  int32_t day;
  int8_t hour, minute, second;

  if (*length < SIZE_ISO_8601) {
    *length = SIZE_ISO_8601;
    return E_BUFFER_TOO_SMALL;
  }

  second = timestamp % 60;
  timestamp /= 60; // now it is minutes

  minute = timestamp % 60;
  timestamp /= 60; // now it is hours

  hour = timestamp % 24;
  timestamp /= 24; // now it is days

  year = 0;
  day = 0;
  while ((day += days_in_year(year)) <= timestamp) {
    year++;
  }
  day -= days_in_year(year);
  timestamp -= day; // now it is days in this year, starting at 0

  day = 0;
  month_length = 0;
  for (month = 0; month < 12; month++) {
    if (month == 1) {
      // February
      month_length = is_leap_year(year) ? 29 : 28;
    } else {
      month_length = MONTH_DAYS[month];
    }

    if (timestamp >= month_length) {
      timestamp -= month_length;
    } else {
      break;
    }
  }
  year = 1970 + year;
  month++; // offset by 1
  day = timestamp + 1;

  int i = 0, j = 0;

  // NOTE: It seems the snprintf implementation in Arduino has bugs,
  // we have to manually piece the string together here.
#define INT_TO_STR(v_, width_) \
  for (j = 0; j < (width_); j++) { \
    buffer[i + (width_) - 1 - j] = '0' + ((v_) % 10); \
    (v_) /= 10; \
  } \
  i += (width_)

  INT_TO_STR(year, 4);
  buffer[i++] = '-';
  INT_TO_STR(month, 2);
  buffer[i++] = '-';
  INT_TO_STR(day, 2);
  buffer[i++] = 'T';
  INT_TO_STR(hour, 2);
  buffer[i++] = ':';
  INT_TO_STR(minute, 2);
  buffer[i++] = ':';
  INT_TO_STR(second, 2);
  buffer[i++] = '.';
  INT_TO_STR(milli, 3);
  buffer[i++] = 'Z';
  buffer[i++] = '\0';

#undef INT_TO_STR

  *length = i;
  return E_OK;
}

/* Reader functions */
#ifdef M2X_ENABLE_READER

// Data structures and functions used to parse stream values

#ifndef M2X_STREAM_BUF_LEN
#define M2X_STREAM_BUF_LEN 32
#endif  /* M2X_STREAM_BUF_LEN */

typedef struct {
  uint8_t state;
  char at_str[M2X_STREAM_BUF_LEN + 1];
  char value_str[M2X_STREAM_BUF_LEN + 1];
  int index;

  stream_value_read_callback callback;
  void* context;
} stream_parsing_context_state;

#define WAITING_AT 0x1
#define GOT_AT 0x2
#define WAITING_VALUE 0x4
#define GOT_VALUE 0x8

#define GOT_STREAM (GOT_AT | GOT_VALUE)
#define TEST_GOT_STREAM(state_) (((state_) & GOT_STREAM) == GOT_STREAM)

#define TEST_IS_AT(state_) (((state_) & (WAITING_AT | GOT_AT)) == WAITING_AT)
#define TEST_IS_VALUE(state_) (((state_) & (WAITING_VALUE | GOT_VALUE)) == \
                               WAITING_VALUE)

static void on_stream_key_found(jsonlite_callback_context* context,
                                jsonlite_token* token)
{
  stream_parsing_context_state* state =
      (stream_parsing_context_state*) context->client_state;
  if (strncmp((const char*) token->start, "timestamp", 9) == 0) {
    state->state |= WAITING_AT;
  } else if ((strncmp((const char*) token->start, "value", 5) == 0) &&
             (token->start[5] != 's')) { // get rid of "values"
    state->state |= WAITING_VALUE;
  }
}

static void on_stream_value_found(jsonlite_callback_context* context,
                                  jsonlite_token* token,
                                  int type)
{
  stream_parsing_context_state* state =
      (stream_parsing_context_state*) context->client_state;

  if (TEST_IS_AT(state->state)) {
    strncpy(state->at_str, (const char*) token->start,
            MIN(token->end - token->start, M2X_STREAM_BUF_LEN));
    state->at_str[MIN(token->end - token->start, M2X_STREAM_BUF_LEN)] = '\0';
    state->state |= GOT_AT;
  } else if (TEST_IS_VALUE(state->state)) {
    strncpy(state->value_str, (const char*) token->start,
            MIN(token->end - token->start, M2X_STREAM_BUF_LEN));
    state->value_str[MIN(token->end - token->start, M2X_STREAM_BUF_LEN)] = '\0';
    state->state |= GOT_VALUE;
  }

  if (TEST_GOT_STREAM(state->state)) {
    state->callback(state->at_str, state->value_str,
                    state->index++, state->context, type);
    state->state = 0;
  }
}

static void on_stream_string_found(jsonlite_callback_context* context,
                                   jsonlite_token* token)
{
  on_stream_value_found(context, token, 1);
}

static void on_stream_number_found(jsonlite_callback_context* context,
                                   jsonlite_token* token)
{
  on_stream_value_found(context, token, 2);
}

// Data structures and functions used to parse locations

#define LOCATION_BUF_LEN 20

typedef struct {
  uint16_t state;
  char name_str[LOCATION_BUF_LEN + 1];
  double latitude;
  double longitude;
  double elevation;
  char timestamp_str[LOCATION_BUF_LEN + 1];
  int index;

  location_read_callback callback;
  void* context;
} location_parsing_context_state;

#define WAITING_NAME 0x1
#define WAITING_LATITUDE 0x2
#define WAITING_LONGITUDE 0x4
#define WAITING_ELEVATION 0x8
#define WAITING_TIMESTAMP 0x10

#define GOT_NAME 0x20
#define GOT_LATITUDE 0x40
#define GOT_LONGITUDE 0x80
#define GOT_ELEVATION 0x100
#define GOT_TIMESTAMP 0x200

#define GOT_LOCATION (GOT_NAME | GOT_LATITUDE | GOT_LONGITUDE | GOT_ELEVATION | GOT_TIMESTAMP)
#define TEST_GOT_LOCATION(state_) (((state_) & GOT_LOCATION) == GOT_LOCATION)

#define TEST_IS_NAME(state_) (((state_) & (WAITING_NAME | GOT_NAME)) == WAITING_NAME)
#define TEST_IS_LATITUDE(state_) (((state_) & (WAITING_LATITUDE | GOT_LATITUDE)) \
                                  == WAITING_LATITUDE)
#define TEST_IS_LONGITUDE(state_) (((state_) & (WAITING_LONGITUDE | GOT_LONGITUDE)) \
                                   == WAITING_LONGITUDE)
#define TEST_IS_ELEVATION(state_) (((state_) & (WAITING_ELEVATION | GOT_ELEVATION)) \
                                   == WAITING_ELEVATION)
#define TEST_IS_TIMESTAMP(state_) (((state_) & (WAITING_TIMESTAMP | GOT_TIMESTAMP)) \
                                   == WAITING_TIMESTAMP)

static void on_location_key_found(jsonlite_callback_context* context,
                                  jsonlite_token* token) {
  location_parsing_context_state* state =
      (location_parsing_context_state*) context->client_state;
  if (strncmp((const char*) token->start, "waypoints", 9) == 0) {
    // only parses those locations in waypoints, skip the outer one
    state->state = 0;
  } else if (strncmp((const char*) token->start, "name", 4) == 0) {
    state->state |= WAITING_NAME;
  } else if (strncmp((const char*) token->start, "latitude", 8) == 0) {
    state->state |= WAITING_LATITUDE;
  } else if (strncmp((const char*) token->start, "longitude", 9) == 0) {
    state->state |= WAITING_LONGITUDE;
  } else if (strncmp((const char*) token->start, "elevation", 9) == 0) {
    state->state |= WAITING_ELEVATION;
  } else if (strncmp((const char*) token->start, "timestamp", 9) == 0) {
    state->state |= WAITING_TIMESTAMP;
  }
}

static void on_location_string_found(jsonlite_callback_context* context,
                                     jsonlite_token* token) {
  location_parsing_context_state* state =
      (location_parsing_context_state*) context->client_state;

  if (TEST_IS_NAME(state->state)) {
    strncpy(state->name_str, (const char*) token->start,
            MIN(token->end - token->start, LOCATION_BUF_LEN));
    state->name_str[MIN(token->end - token->start, LOCATION_BUF_LEN)] = '\0';
    state->state |= GOT_NAME;
  } else if (TEST_IS_LATITUDE(state->state)) {
    state->latitude = atof((const char*) token->start);
    state->state |= GOT_LATITUDE;
  } else if (TEST_IS_LONGITUDE(state->state)) {
    state->longitude = atof((const char*) token->start);
    state->state |= GOT_LONGITUDE;
  } else if (TEST_IS_ELEVATION(state->state)) {
    state->elevation = atof((const char*) token->start);
    state->state |= GOT_ELEVATION;
  } else if (TEST_IS_TIMESTAMP(state->state)) {
    strncpy(state->timestamp_str, (const char*) token->start,
            MIN(token->end - token->start, LOCATION_BUF_LEN));
    state->timestamp_str[MIN(token->end - token->start, LOCATION_BUF_LEN)] = '\0';
    state->state |= GOT_TIMESTAMP;
  }

  if (TEST_GOT_LOCATION(state->state)) {
    state->callback(state->name_str, state->latitude, state->longitude,
                    state->elevation, state->timestamp_str, state->index++,
                    state->context);
    state->state = 0;
  }
}

#ifndef M2X_COMMAND_BUF_LEN
#define M2X_COMMAND_BUF_LEN 40
#endif  /* M2X_COMMAND_BUF_LEN */

typedef struct {
  uint8_t state;
  char id_str[M2X_COMMAND_BUF_LEN + 1];
  char name_str[M2X_COMMAND_BUF_LEN + 1];
  int index;

  m2x_command_read_callback callback;
  void* context;
} m2x_command_parsing_context_state;

#define M2X_COMMAND_WAITING_ID 0x1
#define M2X_COMMAND_GOT_ID 0x2
#define M2X_COMMAND_WAITING_NAME 0x4
#define M2X_COMMAND_GOT_NAME 0x8

#define M2X_COMMAND_GOT_COMMAND (M2X_COMMAND_GOT_ID | M2X_COMMAND_GOT_NAME)
#define M2X_COMMAND_TEST_GOT_COMMAND(state_) \
  (((state_) & M2X_COMMAND_GOT_COMMAND) == M2X_COMMAND_GOT_COMMAND)

#define M2X_COMMAND_TEST_IS_ID(state_) \
  (((state_) & (M2X_COMMAND_WAITING_ID | M2X_COMMAND_GOT_ID)) == \
   M2X_COMMAND_WAITING_ID)
#define M2X_COMMAND_TEST_IS_NAME(state_) \
  (((state_) & (M2X_COMMAND_WAITING_NAME | M2X_COMMAND_GOT_NAME)) == \
   M2X_COMMAND_WAITING_NAME)

static void m2x_on_command_key_found(jsonlite_callback_context* context,
                                     jsonlite_token* token)
{
  m2x_command_parsing_context_state* state =
      (m2x_command_parsing_context_state*) context->client_state;
  if (strncmp((const char*) token->start, "id", 2) == 0) {
    state->state |= M2X_COMMAND_WAITING_ID;
  } else if (strncmp((const char*) token->start, "name", 4) == 0) {
    state->state |= M2X_COMMAND_WAITING_NAME;
  }
}

static void m2x_on_command_value_found(jsonlite_callback_context* context,
                                       jsonlite_token* token)
{
  m2x_command_parsing_context_state* state =
      (m2x_command_parsing_context_state*) context->client_state;
  if (M2X_COMMAND_TEST_IS_ID(state->state)) {
    strncpy(state->id_str, (const char*) token->start,
            MIN(token->end - token->start, M2X_COMMAND_BUF_LEN));
    state->id_str[MIN(token->end - token->start, M2X_COMMAND_BUF_LEN)] = '\0';
    state->state |= M2X_COMMAND_GOT_ID;
  } else if (M2X_COMMAND_TEST_IS_NAME(state->state)) {
    strncpy(state->name_str, (const char*) token->start,
            MIN(token->end - token->start, M2X_COMMAND_BUF_LEN));
    state->name_str[MIN(token->end - token->start, M2X_COMMAND_BUF_LEN)] = '\0';
    state->state |= M2X_COMMAND_GOT_NAME;
  }

  if (M2X_COMMAND_TEST_GOT_COMMAND(state->state)) {
    state->callback(state->id_str, state->name_str,
                    state->index++, state->context);
    state->state = 0;
  }
}

int M2XStreamClient::listStreamValues(const char* deviceId, const char* streamName,
                                      stream_value_read_callback callback, void* context,
                                      const char* query) {
  if (_client->connect(_host, _port)) {
    DBGLN("%s", "Connected to M2X server!");
    _client->print("GET ");
    if (_path_prefix) { _client->print(_path_prefix); }
    _client->print("/v2/devices/");
    _client->print(deviceId);
    _client->print("/streams/");
    print_encoded_string(_client, streamName);
    _client->print("/values");

    if (query) {
      if (query[0] != '?') {
        _client->print('?');
      }
      _client->print(query);
    }

    _client->println(" HTTP/1.0");
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

int M2XStreamClient::readLocation(const char* deviceId,
                                  location_read_callback callback,
                                  void* context) {
  if (_client->connect(_host, _port)) {
    DBGLN("%s", "Connected to M2X server!");
    _client->print("GET ");
    if (_path_prefix) { _client->print(_path_prefix); }
    _client->print("/v2/devices/");
    _client->print(deviceId);
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

int M2XStreamClient::listCommands(const char* deviceId,
                                  m2x_command_read_callback callback,
                                  void* context,
                                  const char* query) {
  if (_client->connect(_host, _port)) {
    DBGLN("%s", "Connected to M2X server!");
    _client->print("GET ");
    if (_path_prefix) { _client->print(_path_prefix); }
    _client->print("/v2/devices/");
    _client->print(deviceId);
    _client->print("/commands");

    if (query) {
      if (query[0] != '?') {
        _client->print('?');
      }
      _client->print(query);
    }

    _client->println(" HTTP/1.0");
    writeHttpHeader(-1);
  } else {
    DBGLN("%s", "ERROR: Cannot connect to M2X server!");
    return E_NOCONNECTION;
  }
  int status = readStatusCode(false);
  if (status == 200) {
    readCommand(callback, context);
  }

  close();
  return status;
}


int M2XStreamClient::readStreamValue(stream_value_read_callback callback,
                                     void* context) {
  const int BUF_LEN = 64;
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
  cbs.number_found = on_stream_number_found;
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

int M2XStreamClient::readCommand(m2x_command_read_callback callback,
                                 void* context) {
  const int BUF_LEN = 60;
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

  m2x_command_parsing_context_state state;
  state.state = 0;
  state.index = 0;
  state.callback = callback;
  state.context = context;

  jsonlite_parser_callbacks cbs = jsonlite_default_callbacks;
  cbs.key_found = m2x_on_command_key_found;
  cbs.string_found = m2x_on_command_value_found;
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

#endif  /* M2X_ENABLE_READER */

#endif  /* M2XStreamClient_h */
