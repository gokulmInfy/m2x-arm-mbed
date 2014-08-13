ARM mbed M2X API Client
========================

The ARM mbed client library is used to send/receive data to/from [AT&amp;T's M2X service](https://m2x.att.com/) from [mbed LPC1768](http://mbed.org/platforms/mbed-LPC1768/) microcontrollers.

Getting Started
==========================
1. Signup for an [M2X Account](https://m2x.att.com/signup).
2. Obtain your _Master Key_ from the Master Keys tab of your [Account Settings](https://m2x.att.com/account) screen.
2. Create your first [Data Source Blueprint](https://m2x.att.com/blueprints) and copy its _Feed ID_.
3. Review the [M2X API Documentation](https://m2x.att.com/developer/documentation/overview).
4. Obtain an [mbed LPC1768](http://mbed.org/platforms/mbed-LPC1768/) and an [mbed application board](http://mbed.org/cookbook/mbed-application-board).

**NOTE**: Though [similiar boards](http://mbed.org/cookbook/Homepage) exist and may also do the job, this doc is specific to using a combination of mbed LPC1768 microcontroller and mbed application board

Please consult the [M2X glossary](https://m2x.att.com/developer/documentation/glossary) if you have questions about any M2X specific terms.

How to run the examples
=======================

To run the examples, please follow the steps below:

1. Launch [mbed online compiler](https://mbed.org/compiler/) in your browser
2. Click `New` to create a new program, select `Empty Program` as the template and give it a name, then click `OK`.
3. Right click on the project you just created, select `Import Library…->From Import Wizard…`, select `Libraries` tab, use the scrollbar or the search box at the right to find `mbed` library, double click it to import it.
4. Use the same step as #3 to import `mbed-rtos` and `EthernetInterface` (mbed official) libraries.

    **NOTE**: To Run the `PostExample` example, you also need to import `LM75B` library in the same way to use the temperature sensor.

5. Right click on the project, select `Import Library…->From URL…`, paste the following URL into `Source URL` box:

    ```
    http://mbed.org/users/citrusbyte/code/jsonlite/
    ```

	Click `Import` to import jsonlite library.
7. Use the same step as #5 to import the following M2X client library:

    ```
    https://mbed.org/teams/ATT-M2X-team/code/M2XStreamClient/
    ```

8. Create a file named `main.cpp`, and paste in any of the examples. Modify the M2X API Key, feed ID or stream name as needed by the examples.
9. When you are done, click `Compile`. if no error is found, a bin file will be downloaded to your computer.
10. Copy that bin file to your mbed microcontroller, press the `reset` button, then you should be able to run the program!

Variables used in Examples
==========================

In order to run the given examples, different variables need to be configured. We will walk through those variables in this section.

M2X API Key
-----------

Once you [register](https://m2x.att.com/signup) for an AT&amp;T M2X account, an API key is automatically generated for you. This key is called a _Primary Master Key_ and can be found in the _Master Keys_ tab of your [Account Settings](https://m2x.att.com/account). This key cannot be edited nor deleted, but it can be regenerated. It will give you full access to all APIs.

However, you can also create a _Data Source API Key_ associated with a given Data Source(Feed), you can use the Data Source API key to access the streams belonging to that Data Source.

You can customize this variable in the following line in the examples:

```
char m2xKey[] = "<M2X access key>";
```

Feed ID
-------

A feed is associated with a data source, it is a set of data streams, such as streams of locations, temperatures, etc. The following line is needed to configure the feed used:

```
char feedId[] = "<feed id>";
```

Stream Name
------------

A stream in a feed is a set of timed series data of a specific type(i,e. humidity, temperature), you can use the M2XStreamClient library to send stream values to M2X server, or receive stream values from M2X server. Use the following line to configure the stream if needed:

```
char streamName[] = "<stream name>";
```

Using the M2XStreamClient library
=========================

In the M2XStreamClient, 4 types of API functions are provided here:

* `send`: Send stream value to M2X server
* `receive`: Receive stream value from M2X server
* `updateLocation`: Send location value of a feed to M2X server
* `readLocation`: Receive location values of a feed from M2X server

Returned values
---------------

For all those functions, the HTTP status code will be returned if we can fulfill a HTTP request. For example, `200` will be returned upon success, `401` will be returned if we didn't provide a valid M2X API Key.

Otherwise, the following error codes will be used:

```
static const int E_NOCONNECTION = -1;
static const int E_DISCONNECTED = -2;
static const int E_NOTREACHABLE = -3;
static const int E_INVALID = -4;
static const int E_JSON_INVALID = -5;
```

Post stream value
-----------------

The following functions can be used to post value to a stream, which belongs to a feed:

```
template <class T>
int post(const char* feedId, const char* streamName, T value);
```

Here we use C++ templates to generate functions for different types of values, feel free to use values of `float`, `int`, `long` or even `const char*` types here.

Post multiple values
--------------------

M2X also supports posting multiple values to multiple streams in one call, use the following function for this:

```
template <class T>
int postMultiple(const char* feedId, int streamNum,
                 const char* names[], const int counts[],
                 const char* ats[], T values[]);
```

Please refer to the comments in the source code on how to use this function, basically, you need to provide the list of streams you want to post to, and values for each stream.

Fetch stream value
------------------

Since mbed microcontroller contains very limited memory, we cannot put the whole returned string in memory, parse it into JSON representations and read what we want. Instead, we use a callback-based mechanism here. We parse the returned JSON string piece by piece, whenever we got a new stream value point, we will call the following callback functions:

```
void (*stream_value_read_callback)(const char* at,
                                   const char* value,
                                   int index,
                                   void* context);

```

The implementation of the callback function is left for the user to fill in, you can read the value of the point in the `value` argument, and the timestamp of the point in the `at` argument. We even pass the index of this this data point in the whole stream as well as a user-specified context variable to this function, so as you can perform different tasks on this.

To read the stream values, all you need to do is calling this function:

```
int fetchValues(const char* feedId, const char* streamName,
                stream_value_read_callback callback, void* context,
                const char* startTime = NULL, const char* endTime = NULL,
                const char* limit = NULL);
```

Besides the feed ID and stream name, only the callback function and a user context needs to be specified. Optional filtering parameters such as start time, end time and limits per call can also be used here.

Update Datasource Location
--------------------------

You can use the following function to update the location for a data source(feed):

```
template <class T>
int updateLocation(const char* feedId, const char* name,
                   T latitude, T longitude, T elevation);
```

Different from stream values, locations are attached to feeds rather than streams. We use templates here, since the values may be in different format, for example, you can express latitudes in both `double` and `const char*`.

Read Datasource Location
------------------------

Similar to reading stream values, we also use callback functions here. The only difference is that different parameters are used in the function:

```
void (*location_read_callback)(const char* name,
                               double latitude,
                               double longitude,
                               double elevation,
                               const char* timestamp,
                               int index,
                               void* context);

```

For memory space consideration, now we only provide double-precision when reading locations. An index of the location points is also provided here together with a user-specified context.

The API is also slightly different, in that the stream name is not needed here:

```
int readLocation(const char* feedId, location_read_callback callback,
                 void* context);

```


How to read Serial output
=========================

Though you can use `printf` on a mbed microcontroller, the output is sent to Serial output. To check what your microcontroller outputs to serial ports, follow the instructions at [here](http://mbed.org/handbook/SerialPC).

Examples
========

We provide a series of examples that will help you get an idea of how to use the `M2XStreamClient` library to perform all kinds of tasks.

Note that the examples contain fictionary variables, and that they need to be configured as per the instructions above before running on your mbed microcontroller. Each of the examples here also needs an Internet connection setup, either via an Ethernet port or a Wifi module.

In the `Post` example, the temperature sensor on the mbed application board is needed. For other examples, no extra modules are needed besides the network connection.

Post
----

This example shows how to post temperatures to M2X server. Before running this, you need to have a valid M2X Key, a feed ID and a stream name. The mbed microcontroller need to be hooked on the mbed application board in order to use the temperature sensor.

PostMultiple
------------

This example shows how to post multiple values to multiple streams in one API call.

FetchValues
-----------

This example reads stream values from M2X server. And prints the stream data point got to Serial interface. You can find the actual values in the Serial output.

UpdateLocation
--------------

This one sends location data to M2X server. Idealy a GPS device should be used here to read the cordinates, but for simplicity, we just use pre-set values here to show how to use the API.

ReadLocation
------------

This one reads location data of a feed from M2X server, and prints them to Serial interfact. You can check the output in the Serial output.

License
=======

This library is released under the MIT license. See [`M2XStreamClient/LICENSE`](M2XStreamClient/LICENSE) for the terms.
