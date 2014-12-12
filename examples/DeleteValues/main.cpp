#include <jsonlite.h>
#include "M2XStreamClient.h"

#include "mbed.h"
#include "EthernetInterface.h"

char deviceId[] = "<device id>"; // Device you want to delete values
char streamName[] = "<stream name>"; // Stream you want to delete values
char m2xKey[] = "<M2X access key>"; // Your M2X access key

char fromTime[] = "<from date/time>"; // yyyy-mm-ddTHH:MM:SS.SSSZ
char endTime[] = "<end date/time>"; // yyyy-mm-ddTHH:MM:SS.SSSZ

Client client;
M2XStreamClient m2xClient(&client, m2xKey);

EthernetInterface eth;

int main() {
  eth.init();
  eth.connect();
  printf("IP Address: %s\n", eth.getIPAddress());

  // Delete values
  int response = m2xClient.deleteValues(deviceId,
                                        streamName,
                                        fromTime,
                                        endTime);
  printf("Delete response code: %d\n", response);

  while (true) {
    delay(5000);
  }
}
