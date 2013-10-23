#include <jsonlite.h>
#include "M2XStreamClient.h"

#include "mbed.h"
#include "EthernetInterface.h"

char feedId[] = "<feed id>"; // Feed you want to post to
char streamName[] = "<stream name>"; // Stream you want to post to
char m2xKey[] = "<m2x key>"; // Your M2X access key

Client client;
M2XStreamClient m2xClient(&client, m2xKey);

EthernetInterface eth;

void on_data_point_found(const char* at, const char* value, int index, void* context) {
  printf("Found a data point, index: %d\n", index);
  printf("At: %s\n Value: %s\n", at, value);
}

int main() {
  eth.init();
  eth.connect();
  printf("IP Address: %s\n", eth.getIPAddress());

  while (true) {
    int response = m2xClient.receive(feedId, streamName, on_data_point_found, NULL);
    printf("Post response code: %d\n", response);

    if (response == -1) while (true) ;

    delay(5000);
  }
}
