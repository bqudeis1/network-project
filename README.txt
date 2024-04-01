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


AREA: convert the areas (SQRMT, SQRML, SQRIN, and SQRFT):
Example:
SQRMT SQRML 15

VOL: conver the volumes (LTR, GALNU, GALNI, and CUBM):
Example:
CUBM LTR 2

WEIGHT: convert the weights (KILO, PND, and CART):
Example:
KILO PND 58

TEMP: convert the temperatures (CELS, FAHR, and KELV):
Example:
FAHR CELS 24

BYE: causes the client to gracefully exit the conncetion with the server
Example:
BYE zani

NOTE (INCLUDIGN LIMITATIONS): 
- TO END THE SERVER CONNECTION, NEED TO PRESS CTRL+C
- DOES NOT HAVE MULTITHREADING
- DOES NOT IMPLEMENT THE CALCULATOR FUNCTIONS BUT CAN SWITCH MODES
- BYE DOESN'T WORK AFTER MODE IS INITIALIED
