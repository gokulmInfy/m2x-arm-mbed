#include <jsonlite.h>
#include "M2XStreamClient.h"

#include "mbed.h"
#include "LM75B.h"
#include "EthernetInterface.h"

char deviceId[] = "<device id>"; // Device you want to push to
char streamName[] = "<stream name>"; // Stream you want to push to
char m2xKey[] = "<m2x api key>"; // Your M2X API Key or Master API Key

Client client;
M2XStreamClient m2xClient(&client, m2xKey);
TimeService timeService(&m2xClient);

EthernetInterface eth;
LM75B tmp(p28,p27);

int main() {
  eth.init();
  eth.connect();
  printf("IP Address: %s\n", eth.getIPAddress());

  if (!m2x_status_is_success(timeService.init())) {
    printf("Cannot initialize time service!\n");
    return 0;
  }

  while (true) {
    double val = tmp.read();
    printf("Current temperature is: %lf\n", val);

    char timestamp[25];
    int length = 25;
    timeService.getTimestamp(timestamp, &length);

    printf("Current timestamp: %s\n", timestamp);

    const char* names[1];
    names[0] = streamName;

    const char *timestamps[1];
    timestamps[0] = timestamp;
    int count = 1;

    int response = m2xClient.postDeviceUpdates<double>(
        deviceId, 1, names,
        &count, timestamps, &val);
    printf("Response code: %d\n", response);

    if (response == -1) while (true) ;

    delay(5000);
  }
}
