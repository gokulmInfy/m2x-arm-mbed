#include <jsonlite.h>
#include "M2XStreamClient.h"

#include "mbed.h"
#include "EthernetInterface.h"

char deviceId[] = "<device id>"; // Device you want to post to
char m2xKey[] = "<m2x api key>"; // Your M2X API Key or Master API Key

const char *streamNames[] = { "temperature", "humidity" };
int counts[] = { 2, 1 };
const char *ats[] = { "2013-10-11T12:34:56Z", NULL, NULL };
double values[] = { 7.9, 11.2, 6.1 };

Client client;
M2XStreamClient m2xClient(&client, m2xKey);

EthernetInterface eth;

int main() {
  eth.init();
  eth.connect();
  printf("IP Address: %s\n", eth.getIPAddress());

  while (true) {
    int response = m2xClient.postDeviceUpdates(deviceId, 2, streamNames, counts, ats, values);
    printf("Response code: %d\n", response);

    if (response == -1) while (true) ;

    delay(5000);
  }
}
