How to run:
make

Once compiled, begin by instantiating server.cpp
./server server.conf

After this is done, can introduce clients as such (multiple clients as its multithreaded)
./client client.conf

Once this is complete, can enter different commands as show below:
HELO: important for initializing the user 
Example:
HELO zani


HELP: for printing out the help menu for the commands and how to enter them
Example:
HELP

AREA: convert the area with units SQRMT, SQRML, SQRIN, and SQRFT (over 
two inputs and applies to the other modes VOL, WGT, and TEMP):
Example:
AREA
SQRMT SQRML 15

VOL: convert the volume with LTR, GALNU, GALNI, and CUBM:
Example:
VOL
CUBM LTR 2

WGT: convert the weight (KILO, PND, and CART):
Example:
WGT
KILO PND 58

TEMP: convert the temperature (CELS, FAHR, and KELV):
Example:
TEMP
FAHR CELS 24

BYE: causes the client to gracefully exit the conncetion with the server
Example:
BYE zani

NOTE (INCLUDIGN LIMITATIONS): 
- TO END THE SERVER CONNECTION, NEED TO PRESS CTRL+C
- Multithreading present manys errors
