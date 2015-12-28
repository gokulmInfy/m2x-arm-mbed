#ifndef TimeService_h
#define TimeService_h

class M2XStreamClient;

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
  Timer _timer;
};

#endif  /* TimeService_h */
