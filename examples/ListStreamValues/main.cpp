#include <jsonlite.h>
#include "M2XStreamClient.h"

#include "mbed.h"
#include "EthernetInterface.h"

char deviceId[] = "<device id>"; // Device you want to receive values
char streamName[] = "<stream name>"; // Stream you want to receive values
char m2xKey[] = "<m2x api key>"; // Your M2X API Key or Master API Key

Client client;
M2XStreamClient m2xClient(&client, m2xKey);

EthernetInterface eth;

void on_data_point_found(const char* at, const char* value, int index, void* context, int type) {
  printf("Found a data point, index: %d type: %d\n", index, type);
  printf("At: %s\n Value: %s\n", at, value);
}

int main() {
  eth.init();
  eth.connect();
  printf("IP Address: %s\n", eth.getIPAddress());

  while (true) {
    int response = m2xClient.listStreamValues(deviceId, streamName, on_data_point_found, NULL);
    printf("Response code: %d\n", response);

    if (response == -1) while (true) ;

    delay(5000);
  }
}
