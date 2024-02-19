#include <iostream>
using namespace std;

// Function declarations
float convertArea(float value, int fromUnit, int toUnit);
float convertVolume(float value, int fromUnit, int toUnit);
float convertWeight(float value, int fromUnit, int toUnit);
float convertTemperature(float value, int fromUnit, int toUnit);

int main() { // IMPLEMENT REDUNDANT I.E. FROM AND TO 1 
    int mode;
    float value;
    int fromUnit, toUnit;
    bool cont = false;

    while (!cont) {
        cout << "\n************* SELECT A MODE *************";
        cout << "\n1. AREA\t\t\t\t3. WGT\n2. VOL\t\t\t\t4. TEMP\n\t\t5. Exit";
        cout << "\nEnter one of the following modes (1-4) or Exit (5): ";
        cin >> mode;
        if (mode >= 1 && mode <= 5) {
            cont = 1;
        }
    } cont = 0;

    while (!cont) {
        cout << "\n\n************* METRIC SELECTION *************\n";
        switch (mode) {
            case 1: // AREA
                cout << "1. Square Meters\t\t3. Square Inches\n2. Square Miles\t\t4. Square Foot\n5. Exit \t\t6. Switch Modes\n";
                break;
            case 2: // VOL
                cout << "1. Liters\t\t3. Imperial Gallons\n2. US Gallons\t\t4. Cubic Meters\n";
                break;
            case 3: // WGT
                cout << "1. Kilograms\n2. Pounds\n3. Carats\n";
                break;
            case 4: // TEMP
                cout << "1. Celsius\n2. Fahrenheit\n3. Kelvin\n";
                break;
            case 5: // TEMP
                cout << "\n";
                break;
            case 6: // TEMP
                cout << "1. Celsius\n2. Fahrenheit\n3. Kelvin\n";
                break;
            default: // If none of the above
                cout << "Invalid mode\n";
                return 0;
        }

        if (mode == 1 || mode == 2) { // Modes 1 & 2
            cout << "Enter value to convert (1-4): ";
            cin >> fromUnit;

            cout << "Enter desired metric (1-4): ";
            cin >> toUnit;

            cout << "\nEnter value you wish to convert: ";
            cin >> value;

            if (fromUnit >= 1 && fromUnit <= 4 && toUnit >= 1 && toUnit <= 4) {
                cont = 1;
            }
        }
        else if (mode == 3 || mode == 4) { // Modes 3 & 4
            cout << "Enter value to convert (1-3): ";
            cin >> fromUnit;

            cout << "Enter desired metric (1-3): ";
            cin >> toUnit;   

            cout << "\nEnter value you wish to convert: ";
            cin >> value;

            if (fromUnit >= 1 && fromUnit <= 3 && toUnit >= 1 && toUnit <= 3) {
                cont = 1;
            }
        }
    } cont = 0;

    // Perform conversion based on the selected mode
    switch (mode) {
        case 1: // AREA
            cout << "Result: " << convertArea(value, fromUnit, toUnit) << endl;
            break;
        case 2: // VOL
            cout << "Result: " << convertVolume(value, fromUnit, toUnit) << endl;
            break;
        case 3: // WGT
            cout << "Result: " << convertWeight(value, fromUnit, toUnit) << endl;
            break;
        case 4: // TEMP
            cout << "Result: " << convertTemperature(value, fromUnit, toUnit) << endl;
            break;
        case 5: // IMPLEMEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEENT
            cout << "Exiting..." << endl;
            break;
        case 6: // IMPLEMEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEENT
            cout << "Switching Modes..." << endl;
            break;
        default:
            cout << "Invalid mode\n";
            return 1;
    }

    return 0;
}

// Conversion functions
float convertArea(float value, int fromUnit, int toUnit) {
    switch (fromUnit) { // SQRMT SQRML SQRIN SQRFT 
        case 1:
            switch (toUnit) {
                case 2:
                    return (value / 2589988.11);
                    break;
                case 3:
                    return (value * 1550);
                    break;
                case 4:
                    return (value * 10.764);
                    break;
            }
            break;
        case 2:
            switch (toUnit) {
                case 1:
                    return (value * 2589988.11);
                    break;
                case 3:
                    return (value * 4014489600);
                    break;
                case 4:
                    return (value * 27878555.87);
                    break;
            }
            break;
        case 3:
            switch (toUnit) {
                case 1:
                    return (value / 1550);
                    break;
                case 2:
                    return (value / 4014489600);
                    break;
                case 4:
                    return (value / 144);
                    break;
            }
            break;  
        case 4:
            switch (toUnit) {
                case 1:
                    return (value / 10.764);
                    break;
                case 2:
                    return (value / 27878555.87);
                    break;
                case 3:
                    return (value * 144);
                    break;
            }
            break;   
    }
}

float convertVolume(float value, int fromUnit, int toUnit) {
    switch (fromUnit) { // Liters, US Gallons, Imperial Gallons, Cubic Meters
        case 1:
            switch (toUnit) {
                case 2:
                    return (value / 3.785);
                    break;
                case 3:
                    return (value / 4.546);
                    break;
                case 4:
                    return (value / 1000);
                    break;
                }
            break;
        case 2:
            switch (toUnit) {
                case 1:
                    return (value * 3.785);
                    break;
                case 3:
                    return (value * 0.83267384);
                    break;
                case 4:
                    return (value / 264.2);
                    break;
            }
            break;
        case 3:
            switch (toUnit) {
                case 1:
                    return (value * 4.546);
                    break;
                case 2:
                    return (value / 0.83267384);
                    break;
                case 4:
                    return (value / 220);
                    break;
            }
            break;  
        case 4:
            switch (toUnit) {
                case 1:
                    return (value * 1000);
                    break;
                case 2:
                    return (value * 264.2);
                    break;
                case 3:
                    return (value * 220);
                    break;
            }
            break;   
    }
}

float convertWeight(float value, int fromUnit, int toUnit) {
    switch (fromUnit) { // Kilograms, Pounds, and Carats
        case 1:
            switch (toUnit) {
                case 2:
                    return (value * 2.205);
                    break;
                case 3:
                    return (value * 5000);
                    break;
                }
            break;
        case 2:
            switch (toUnit) {
                case 1:
                    return (value / 2.205);
                    break;
                case 3:
                    return (value * 2268);
                    break;
            }
            break;
        case 3:
            switch (toUnit) {
                case 1:
                    return (value / 5000);
                    break;
                case 2:
                    return (value / 2268);
                    break;
            }
            break;    
    }
}

float convertTemperature(float value, int fromUnit, int toUnit) {
    switch (fromUnit) { // Celsius, Fahrenheit, and Kelvin
        case 1:
            switch (toUnit) {
                case 2:
                    return ((value / 1.8) + 32);
                    break;
                case 3:
                    return (value  + 273.15);
                    break;
                }
            break;
        case 2:
            switch (toUnit) {
                case 1:
                    return ((value - 32) * (5/9));
                    break;
                case 3:
                    return ( ( (value - 32) * (5/9) ) + 273.15);
                    break;
            }
            break;
        case 3:
            switch (toUnit) {
                case 1:
                    return (value - 273.15);
                    break;
                case 2:
                    return ( ( (value - 273.15) * (9/5) ) + 32);
                    break;
            }
            break;    
    }
}