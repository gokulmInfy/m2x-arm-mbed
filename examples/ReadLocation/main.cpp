#define MBED_PLATFORM
#define M2X_ENABLE_READER

#include <jsonlite.h>
#include "M2XStreamClient.h"

#include "mbed.h"
#include "EthernetInterface.h"

char deviceId[] = "<device id>"; // Device you want to read location
char m2xKey[] = "<m2x api key>"; // Your M2X API Key or Master API Key

Client client;
M2XStreamClient m2xClient(&client, m2xKey);

EthernetInterface eth;

void on_location_found(const char* name,
                       double latitude,
                       double longitude,
                       double elevation,
                       const char* timestamp,
                       int index,
                       void* context) {
  printf("Found a location, index: %d\n", index);
  printf("Name: %s\nLatitude: %lf\nLongitude: %lf\n", name, latitude, longitude);
  printf("Elevation: %lf\nTimestamp: %s\n", elevation, timestamp);
}

int main() {
  eth.init();
  eth.connect();
  printf("IP Address: %s\n", eth.getIPAddress());

  while (true) {
    int response = m2xClient.readLocation(deviceId, on_location_found, NULL);
    printf("Read response code: %d\n", response);

    if (response == -1) while (true) ;

    delay(5000);
  }
}
