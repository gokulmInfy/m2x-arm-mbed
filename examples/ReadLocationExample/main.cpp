#include <jsonlite.h>
#include "M2XStreamClient.h"

#include "mbed.h"
#include "EthernetInterface.h"

char feedId[] = "<feed id>"; // Feed you want to post to
char m2xKey[] = "<m2x key>"; // Your M2X access key

Client client;
M2XStreamClient m2xClient(&client, m2xKey, "192.168.1.4", 9393);

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
    int response = m2xClient.readLocation(feedId, on_location_found, NULL);
    printf("Post response code: %d\n", response);

    if (response == -1) while (true) ;

    delay(5000);
  }
}
