ARM-mbed M2X API Client Document Creation
=====================

This script can be used to generate developer documentation for the ARM-mbed client library.
You need run this script from the /docs folder.
This uses the doxygen tool to generate the documentation from the source code.
Output is a html folder with the generated documentation inside the /docs folder.

This script tries to install doxygen using Homebrew.
To know more about doxygen, visit [Doxygen page](http://www.stack.nl/~dimitri/doxygen/manual/index.html) 
 
Note: Currently the src folder path in the script is set using the INPUT parameter, if needed can be modified accordingly.And the destination path is, by default, the current location.

