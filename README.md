m2x-arm-mbed
============

How to run the examples
=======================

To run the examples, please follow the steps below:

1. Launch [mbed online compiler](https://mbed.org/compiler/) in your browser
2. Click `New` to create a new program, select `Empty Program` as the template and give it a name, then click `OK`.
3. Right click on the project you just created, select `Import Library…->From Import Wizard…`, select `Libraries` tab, use the scrollbar or the search box at the right to find `mbed` library, double click it to import it.
4. Use the same step as #3 to import `mbed-rtos` and `EthernetInterface` libraries.

    **NOTE**: To Run the `PostExample` example, you also need to import `LM75B` library in the same way to use the temperature sensor.

5. Right click on the project, select `Import Library…->From URL…`, paste the following URL into `Source URL` box:

    ```
    http://mbed.org/users/defmacro/code/jsonlite/
    ```

	Click `Import` to import jsonlite library.
7. Open the Import Wizard as in step #3 again, this time click `Upload` tab, add the `M2XStreamClient.zip` file in current folder  using the file select tag at the bottom, then click `Import`. At the `Import Name` field, keep the name `M2XStreamClient`, in the `Target Path` dropdown menu, select the corrent project, click `Import` to import M2X library.
8. Create a file named `main.cpp`, and paste in any of the examples. Modify the M2X key, feed ID or stream name as needed by the examples.
9. When you are done, click `Compile`. if no error is found, a bin file will be downloaded to your computer.
10. Copy that bin file to your mbed board, press the `reset` button, then you should be able to run the program!