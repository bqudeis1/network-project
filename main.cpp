#include <iostream>
using namespace std;

// Function declarations
float convertArea(float value, string fromUnit, string toUnit);
float convertVolume(float value, string fromUnit, string toUnit);
float convertWeight(float value, string fromUnit, string toUnit);
float convertTemperature(float value, string fromUnit, string toUnit);

// Function declarations
string modeSelection();
void metricSelection(string mode, string& fromUnit, string& toUnit, float& value);

int main() {
    float value;
    string mode, fromUnit, toUnit;
    bool running = true;

    while (running) {
        mode = modeSelection();

        if (mode == "EXIT") {
            running = false;
        }

        if (running) {
            metricSelection(mode, fromUnit, toUnit, value);

            // Perform conversion based on the selected mode
            if (mode == "AREA") {
                cout << "Result: " << convertArea(value, fromUnit, toUnit) << endl;
            } 
            else if (mode == "VOL") {
                cout << "Result: " << convertVolume(value, fromUnit, toUnit) << endl;
            } 
            else if (mode == "WGT") {
                cout << "Result: " << convertWeight(value, fromUnit, toUnit) << endl;
            } 
            else if (mode == "TEMP") {
                cout << "Result: " << convertTemperature(value, fromUnit, toUnit) << endl;
            } 
            else if (mode == "EXIT") {
                cout << "Exiting..." << endl;
            } 
            else if (mode == "SWITCH") {
                cout << "SWITCHING MODES" << endl;
            } 
            else {
                cout << "Invalid mode\n";
            }
        }
    }

    return 0;
}

string modeSelection() {
    string mode;
    bool cont = false;
    while (!cont) {
        cout << "\n************* SELECT A MODE *************";
        cout << "\n1. AREA\t\t\t\t4. TEMP\n2. VOL\t\t\t\t5. EXIT\n3. WGT";
        cout << "\n\nEnter one of the following modes (i.e. AREA): ";
        cin >> mode;
        if (mode == "AREA" || mode == "VOL" || mode == "WGT" || mode == "TEMP" || mode == "EXIT") {
            cont = true;
        }
    }
    return mode;
}

void metricSelection(string mode, string& fromUnit, string& toUnit, float& value) {
    bool cont = false, valid = false;
    while (!cont) {
        cout << "\n\n************* METRIC SELECTION *************\n";

        if (mode == "AREA") {
            cout << "1. SQRMT\t\t\t3. SQRIN\n2. SQRML\t\t\t4. SQRFT\n6. SWITCH\t\t\t5. EXIT\n\n";
            valid = true;
        } 
        else if (mode == "VOL") {
            cout << "1. LTR\t\t\t3. GALNI\n2. GALNU\t\t\t4. CUBM\n6. SWITCH\t\t\t5. EXIT\n\n";
            valid = true;        
        } 
        else if (mode == "WGT") {
            cout << "1. KILO\t\t\t3. CART\n2. PND\t\t\t4. CUBM\n5. EXIT\n\n";
            valid = true;         
        } 
        else if (mode == "TEMP") {
            cout << "1. CELS\t\t\t3. KELV\n2. FAHR\t\t\t4. CUBM\n5. EXIT\n\n";
            valid = true; 
        } else {
            cout << "Invalid mode\n";
            return;
        }

        if (valid) {
            cout << "\nEnter original metric, desired metric, and value (i.e. SQRMT SQRML 150): ";
            cin >> fromUnit >> toUnit >> value;
        }

        // exit the while loop
        cont = true;  
    }
}

// convert area SQRMT, SQRML, SQRIN, and SQRFT
float convertArea(float value, string fromUnit, string toUnit) {
    if (fromUnit == "SQRMT") {
        if (toUnit == "SQRML") {
            return (value / 2589988.11);
        } 
        else if (toUnit == "SQRIN") {
            return (value * 1550);
        } 
        else if (toUnit == "SQRFT") {
            return (value * 10.764);
        }
    } 
    else if (fromUnit == "SQRML") {
        if (toUnit == "SQRMT") {
            return (value * 2589988.11);
        } 
        else if (toUnit == "SQRIN") {
            return (value * 4014489600);
        } 
        else if (toUnit == "SQRFT") {
            return (value * 27878555.87);
        }
    } 
    else if (fromUnit == "SQRIN") {
        if (toUnit == "SQRMT") {
            return (value / 1550);
        } 
        else if (toUnit == "SQRML") {
            return (value / 4014489600);
        } 
        else if (toUnit == "SQRFT") {
            return (value / 144);
        }
    } 
    else if (fromUnit == "SQRFT") {
        if (toUnit == "SQRMT") {
            return (value / 10.764);
        } 
        else if (toUnit == "SQRML") {
            return (value / 27878555.87);
        } 
        else if (toUnit == "SQRIN") {
            return (value * 144);
        }
    }
}

// convert volume LTR, GALNU, GALNI, and CUBM
float convertVolume(float value, string fromUnit, string toUnit) {
    if (fromUnit == "LTR") {
        if (toUnit == "GALNU") {
            return (value / 3.785);
        } 
        else if (toUnit == "GALNI") {
            return (value / 4.546);
        } 
        else if (toUnit == "CUBM") {
            return (value / 1000);
        }
    } else if (fromUnit == "GALNU") {
        if (toUnit == "LTR") {
            return (value * 3.785);
        } 
        else if (toUnit == "GALNI") {
            return (value * 0.83267384);
        } 
        else if (toUnit == "CUBM") {
            return (value / 264.2);
        }
    } else if (fromUnit == "GALNI") {
        if (toUnit == "LTR") {
            return (value * 4.546);
        } 
        else if (toUnit == "GALNU") {
            return (value / 0.83267384);
        } 
        else if (toUnit == "CUBM") {
            return (value / 220);
        }
    } else if (fromUnit == "CUBM") {
        if (toUnit == "LTR") {
            return (value * 1000);
        } 
        else if (toUnit == "GALNU") {
            return (value * 264.2);
        } 
        else if (toUnit == "GALNI") {
            return (value * 220);
        }
    }
}

// convert weight KILO PND CART
float convertWeight(float value, string fromUnit, string toUnit) {
    if (fromUnit == "KILO") {
        if (toUnit == "PND") { 
            return (value * 2.205);
        } 
        else if (toUnit == "CART") { 
            return (value * 5000);
        }
    } 
    else if (fromUnit == "PND") { 
        if (toUnit == "KILO") { 
            return (value / 2.205);
        } 
        else if (toUnit == "CART") { 
            return (value * 2268);
        }
    } 
    else if (fromUnit == "CART") { 
        if (toUnit == "KILO") { 
            return (value / 5000);
        } 
        else if (toUnit == "PND") { 
            return (value / 2268);
        }
    }
}

// convert temp CELS FAHR KELV
float convertTemperature(float value, string fromUnit, string toUnit) {
    if (fromUnit == "CELS") {
        if (toUnit == "FAHR") { 
            return ((value / 1.8) + 32);
        } 
        else if (toUnit == "KELV") { 
            return (value  + 273.15);
        }
    } 
    else if (fromUnit == "FAHR") { 
        if (toUnit == "CELS") { 
            return ((value - 32) * (5/9));
        } 
        else if (toUnit == "KELV") { 
            return ( ( (value - 32) * (5/9) ) + 273.15);
        }
    } 
    else if (fromUnit == "KELV") { 
        if (toUnit == "CELS") { 
            return (value - 273.15);
        } 
        else if (toUnit == "FAHR") { 
            return ( ( (value - 273.15) * (9/5) ) + 32);
        }
    } 
}