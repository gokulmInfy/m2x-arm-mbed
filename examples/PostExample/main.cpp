#include <jsonlite.h>
#include "M2XStreamClient.h"

#include "mbed.h"
#include "LM75B.h"
#include "EthernetInterface.h"

char feedId[] = "<feed id>"; // Feed you want to post to
char streamName[] = "<stream name>"; // Stream you want to post to
char m2xKey[] = "<m2x api key>"; // Your M2X API Key or Master API Key

Client client;
M2XStreamClient m2xClient(&client, m2xKey);

EthernetInterface eth;
LM75B tmp(p28,p27);

int main() {
  eth.init();
  eth.connect();
  printf("IP Address: %s\n", eth.getIPAddress());

  while (true) {
    double val = tmp.read();
    printf("Current temperature is: %lf", val);

    int response = m2xClient.send(feedId, streamName, val);
    printf("Post response code: %d\n", response);

    if (response == -1) while (true) ;

    delay(5000);
  }
}
