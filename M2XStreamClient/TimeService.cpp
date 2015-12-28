#include "M2XStreamClient.h"

static int fill_iso8601_timestamp(int32_t seconds, int32_t milli,
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
    // In case of a timestamp overflow, we reset the server timestamp recorded.
    // Notice that unlike Arduino, mbed would overflow every 30 minutes,
    // so if 2 calls to this API are more than 30 minutes apart, we are
    // likely to run into troubles. This is a limitation of the current
    // mbed platform. However, we argue that it might be a rare case that
    // 2 calls to this are 30 minutes apart. In most cases, we would call
    // this API every few seconds or minutes, this won't be a huge problem.
    // However, if you have a use case that would require 2 intervening
    // calls to this be 30 minutes apart, you might want to leverage a RTC
    // clock instead of the simple ticker here.
    int status = reset();
    if (!m2x_status_is_success(status)) { return status; }
    now = _timer.read_ms();
  }
  if (now < _local_last_milli) {
    // We have already reseted the timestamp, so this cannot happen,
    // an HTTP request cannot last 30 minutes, something else must be wrong
    // here.
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

static int fill_iso8601_timestamp(int32_t timestamp, int32_t milli,
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
