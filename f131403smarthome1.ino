#include <Wire.h>
#include <LiquidCrystal.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>
#include <EEPROM.h>

// Define global variables
#define BACKLIGHT_YELLOW 0x3
#define BACKLIGHT_GREEN 0x2
#define BACKLIGHT_PURPLE 0x5
#define BACKLIGHT_WHITE 0x7
#define BUTTON_SELECT 0x01 
#define DEVICE_ADDRESS_START 0
#define DEVICE_SIZE sizeof(Device)

bool selectButtonPressed = false;
bool selectButtonReleased = false;
unsigned long selectButtonPressedTime = 0;
int backlightColor = BACKLIGHT_PURPLE;
bool isSynchronized = false;
bool isDisplayCleared = false;
unsigned long lastSyncTime = 0;
const int MAX_DEVICES = 10;


struct Device {
  String id;
  char type;
  String location;
  bool state;
  int power;
  int temperature;
};

// Define an array of Device structs with size MAX_DEVICES
Device devices[MAX_DEVICES];

// Initialize a variable to keep track of the number of devices currently stored in the devices array
int numDevices = 0;

// This function writes a Device struct to EEPROM at a specific index based on the deviceIndex parameter
// It calculates the address to write to based on the DEVICE_ADDRESS_START and DEVICE_SIZE constants
void writeDeviceToEEPROM(int deviceIndex, Device device) {
int address = DEVICE_ADDRESS_START + deviceIndex * DEVICE_SIZE;
EEPROM.put(address, device);
}

// This function reads a Device struct from EEPROM at a specific index based on the deviceIndex parameter
// It calculates the address to read from based on the DEVICE_ADDRESS_START and DEVICE_SIZE constants
// It returns the Device struct that is read from EEPROM
Device readDeviceFromEEPROM(int deviceIndex) {
int address = DEVICE_ADDRESS_START + deviceIndex * DEVICE_SIZE;
Device device;
EEPROM.get(address, device);
return device;
}

// This function reads all the Devices stored in EEPROM into the devices array
// It starts reading from the address specified by DEVICE_ADDRESS_START and reads DEVICE_SIZE bytes at a time
// It stops reading when it reaches the end of the EEPROM or when it has read MAX_DEVICES devices
// It stores each Device struct in the devices array and updates the numDevices variable accordingly
// If it encounters an empty Device struct (indicated by an empty id), it stops reading
void readDevicesFromEEPROM() {
int deviceIndex = 0;
for (int address = DEVICE_ADDRESS_START; address < EEPROM.length() && deviceIndex < MAX_DEVICES; address += DEVICE_SIZE) {
Device device;
EEPROM.get(address, device);
if (device.id == "") {
// This address does not contain a valid device, stop reading
break;
}
devices[deviceIndex] = device;
deviceIndex++;
}
numDevices = deviceIndex;
}

// Define LCD display object
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

// Define an address in EEPROM to store a flag
#define EEPROM_FLAG_ADDRESS 100

// Function to set the flag in EEPROM to a given value
void setEEPROMFlag(bool value) {
  EEPROM.write(EEPROM_FLAG_ADDRESS, value);
}

// Function to get the flag value from EEPROM
bool getEEPROMFlag() {
  return EEPROM.read(EEPROM_FLAG_ADDRESS);
}

// Setup function runs once when the Arduino is powered on or reset
void setup() {
  // Check if the EEPROM flag is set and initialize it if not
  if (!getEEPROMFlag()) {
    setEEPROMFlag(false);
  }
  
  // Read devices from EEPROM if the flag is set
  if (getEEPROMFlag()) {
    readDevicesFromEEPROM();
  }
  
  // Initialize serial communication
  Serial.begin(9600);
   
  // Initialize LCD display
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setBacklight(BACKLIGHT_PURPLE);
  lcd.print("SMART HOME");

  // Synchronization phase
  while (!isSynchronized) {
    synchronize();
  }

  // Main program loop phase
  lcd.clear();
  lcd.setBacklight(BACKLIGHT_WHITE);
  lcd.print("BASIC\n");
  backlightColor = BACKLIGHT_WHITE;

}

// Function to store devices in EEPROM
void storeDevicesInEEPROM() {
  // Write devices to EEPROM
  for (int i = 0; i < numDevices; i++) {
    writeDeviceToEEPROM(i, devices[i]);
  }

  // Set the EEPROM flag to indicate that devices have been stored
  setEEPROMFlag(true);
}

// Function for synchronizing with a remote system
void synchronize() {
  static int progress = 0;
  static bool progressDirection = true;

  // Send Q character every second
  if (millis() - lastSyncTime > 1000) {
    Serial.write('Q');
    lastSyncTime = millis();
    if (progressDirection) {
      progress++;
    } else {
      progress--;
    }
    if (progress == 15) {
      progressDirection = false;
    } else if (progress == 0) {
      progressDirection = true;
    }
    if (!isDisplayCleared) {
      lcd.clear();
      isDisplayCleared = true;
    }
    lcd.setCursor(0, 1);
    lcd.print("[");
    for (int i = 0; i < 15; i++) {
      if (i < progress) {
        lcd.print("=");
      } else {
        lcd.print(" ");
      }
    }
    lcd.print("]");

  }

  // Check if X character received
  if (Serial.available() > 0) {
    char c = Serial.read();
    if (c == 'X') {
      // Synchronization complete
      isSynchronized = true;
    } else {
      // Error: invalid character received
      lcd.clear();
      lcd.print("Sync error: ");
      lcd.print(c);
      backlightColor = BACKLIGHT_YELLOW;
      lcd.setBacklight(backlightColor);
      delay(500);
      backlightColor = BACKLIGHT_PURPLE;
      lcd.setBacklight(backlightColor);
    }
  }
}

//Declare two variables, __heap_start and *__brkval, as external unsigned integers.
extern unsigned int __heap_start, *__brkval;

//Define a function, FreeRam(), that calculates the amount of free memory on the board by subtracting the address of the 
//free_memory variable from either __heap_start or __brkval, depending on which one is not equal to zero. The function returns the calculated value.
int FreeRam() {
  int free_memory;
  if ((int)__brkval == 0)
    free_memory = ((int)&free_memory) - ((int)&__heap_start);
  else
    free_memory = ((int)&free_memory) - ((int)__brkval);
  return free_memory;
}

void loop() {
  // Read buttons
  byte buttons = lcd.readButtons();

  // Check if SELECT button is pressed
  if (buttons & BUTTON_SELECT) {
    if (!selectButtonPressed) {
      selectButtonPressed = true;
      selectButtonPressedTime = millis();
    }
    if (millis() - selectButtonPressedTime > 1000) { // Check if SELECT button is held for more than one second
      lcd.clear();
      lcd.setBacklight(BACKLIGHT_PURPLE);    
      lcd.print("F131403"); 
      lcd.setCursor(0, 1);
      lcd.print("Free SRAM: ");
      lcd.print(FreeRam());
      selectButtonReleased = false;
      while (buttons & BUTTON_SELECT) {
        buttons = lcd.readButtons();
        if (!selectButtonReleased && !(buttons & BUTTON_SELECT)) {
          selectButtonReleased = true;
          lcd.clear();
          lcd.setBacklight(BACKLIGHT_WHITE);
          lcd.print("BASIC\n");
        }
      }
    }
  } else {
    selectButtonPressed = false;
  }
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    Serial.println(input);
    char message_type = input.charAt(0);
    switch(message_type) {
      case 'A':
        addNewDevice2(input.substring(2)); // skip the 'A-' prefix
        Serial.println(input.substring(2));
        break;
      case 'S':
        updateDeviceState2(input.substring(2)); //skip the 'S-' prefix
        break;
      case 'P':
        handleDevicePower(input.substring(2)); // pass input string without 'P-' prefix
        break;
      case 'R':
        removeDevice(input.substring(2)); // skip the 'R-' prefix
        break;
      case 'X':
        //do nothing
        break;
      default:
        //Invalid message type
        if (message_type != '\r') { // ignore empty lines
              Serial.print("ERROR: Invalid message type '");
              Serial.print(message_type);
              Serial.println("'");
            }
            break;
        }
      }
    }

void printDevice(Device device) {
  String stateString = device.state ? " ON" : "OFF";
  String powerString = device.type == 'S' || device.type == 'L' ? " " + String(device.power) + "%" : "";
  String temperatureString = device.type == 'T' ? " " + String(device.temperature) + "°C" : "";

  byte UP_ARROW[8] = {
    0b00100,
    0b01110,
    0b10101,
    0b00100,
    0b00100,
    0b00100,
    0b00000,
    0b00000
  };

  byte DOWN_ARROW[8] = {
    0b00000,
    0b00000,
    0b00100,
    0b00100,
    0b00100,
    0b10101,
    0b01110,
    0b00100
  };

  lcd.createChar(1, UP_ARROW);
  lcd.createChar(2, DOWN_ARROW);

  lcd.write(byte(1)); // displays the up arrow
  lcd.print(device.id);
  lcd.print(" ");
  lcd.print(device.location.substring(0, 11));
  lcd.print(" ");
  lcd.setCursor(0,1);
  lcd.write(byte(2)); // displays the down arrow
  lcd.print(device.type);
  lcd.print(" ");
  lcd.print(stateString);
  lcd.print(" ");
  lcd.print(powerString);
  lcd.print(" ");
  lcd.print(temperatureString);
  lcd.print("      ");
}


void printDevices() {
  for (int i = 0; i < numDevices; i++) {
    printDevice(devices[i]);
  }
}

void addNewDevice2(String deviceString) {
  #define MAX_TOKENS 3
  #define MAX_LEN 17
  char input[MAX_LEN+1];
  deviceString.toCharArray(input, sizeof(input));  // convert deviceString to a character array
  char tokens[MAX_TOKENS][MAX_LEN+1];  // array to hold the tokens
  int token_count = 0;
  // tokenize the input string
  char *ptr = strtok(input, "-");
  while (ptr != NULL && token_count < MAX_TOKENS) {
    strncpy(tokens[token_count], ptr, MAX_LEN);
    tokens[token_count][MAX_LEN] = '\0';
    token_count++;
    ptr = strtok(NULL, "-");
  }
    // check the length of the second token is 3
    if (strlen(tokens[0]) == 3) {
      // check if the third token is one of 'S', 'T', 'O', 'L', or 'C'
      if (tokens[1][0] == 'S' || tokens[1][0] == 'T' || tokens[1][0] == 'O' || tokens[1][0] == 'L' || tokens[1][0] == 'C') {
        // check if the fourth token is not empty or null
        if (tokens[2] != NULL) {
          String token2 = String(tokens[2]); // Convert char array to String
          token2.trim(); // Trim leading and trailing whitespaces
          if (token2.length() > 0 && token2.length() < 15) {
                // successful location value
                addDevice(tokens[0], tokens[1][0], tokens[2], tokens[3], 0, 0, &numDevices, MAX_DEVICES);
                lcd.clear();
                backlightColor = BACKLIGHT_GREEN;
                lcd.setBacklight(backlightColor);
                delay(500);
                backlightColor = BACKLIGHT_YELLOW;
                lcd.setBacklight(backlightColor);
                Serial.println("New device added: ");
                // print the newly added device
                printDevice(devices[numDevices - 1]);
          } else {
            // tokens[2] is null, empty, has an invalid length or is not trimmed
            Serial.println("ERROR: Missing or empty location information");
          }
        } else {
          // tokens[2] is null
          Serial.println("ERROR: Missing or empty location information");
        }
      } else {
        // wrong device type
        Serial.println("ERROR: Invalid device type");
      }
    } else {
      // invalid ID format
      Serial.println("ERROR: Invalid ID format");
    }


  //Serial.println("ERROR: Maximum number of devices reached");
}

// Function to add a new device to the devices array
void addDevice(const char* id, char type, const char* location, const char* state, int power, int temperature, int* numDevices, int maxDevices) {
  devices[*numDevices].id = id;
  devices[*numDevices].type = type;
  devices[*numDevices].location = location;
  devices[*numDevices].state = (strcmp(state, "ON") == 0);
  if (type == 'S' || type == 'L') {
    devices[*numDevices].power = power;
  } else if (type == 'T') {
    devices[*numDevices].temperature = temperature;
  }
  (*numDevices)++;
}

// Function to find the index of a device in the devices array
int findDeviceIndex(String id) {
  int totaldevices = sizeof(devices) / sizeof(devices[0]);
  for (int i = 0; i < totaldevices; i++) {
    if (id.equals(devices[i].id)) {
      return i;
    }
  }
  return -1;
}

// This function updates the state of a device based on the input string passed as an argument.
//The function first checks if the input string is in a valid format, then tokenizes the string and extracts the necessary information.
//It then finds the index of the device to be updated and updates its state based on the second token.
void updateDeviceState2(String deviceString) {
  if (deviceString.length() < 5) {
    Serial.println("ERROR: Invalid message format");
    return;
  }

  #define MAX_TOKENS_UPDATE 3
  #define MAX_LEN_UPDATE 10

  char input[MAX_LEN_UPDATE+1];
  deviceString.toCharArray(input, sizeof(input));
  char tokens_updatestate[MAX_TOKENS_UPDATE][MAX_LEN_UPDATE+1];
  int token_count1 = 0;

  char *ptr = strtok(input, "-");
  while (ptr != NULL && token_count1 < MAX_TOKENS_UPDATE) {
    strncpy(tokens_updatestate[token_count1], ptr, MAX_LEN_UPDATE);
    tokens_updatestate[token_count1][MAX_LEN_UPDATE-1] = '\0';
    token_count1++;
    ptr = strtok(NULL, "-");
  }

  //find the index of the device
  int deviceIndex_forupdate = findDeviceIndex(tokens_updatestate[0]);
  if (deviceIndex_forupdate != -1) {
    String strDeviceID = tokens_updatestate[1];
    strDeviceID.trim();

    //If the second token is "ON", the device state is set to true and the LCD backlight is set to green.
    //If the second token is "OFF", the device state is set to false and the LCD backlight is set to yellow.
    //If the second token is not "ON" or "OFF", an error message is printed.
    //If the device is not found, a warning message is printed.
    if (strDeviceID.equals("ON")) {
      devices[deviceIndex_forupdate].state = true;
      Serial.println("Device state updated: ");
      lcd.setBacklight(BACKLIGHT_GREEN);
      printDevice(devices[deviceIndex_forupdate]);
    } else if (strDeviceID.equals("OFF")) {
      devices[deviceIndex_forupdate].state = false;
      Serial.println("Device state updated: ");
      lcd.setBacklight(BACKLIGHT_YELLOW);
      printDevice(devices[deviceIndex_forupdate]);
    } else {
      Serial.println("ERROR: Invalid device state");
      Serial.print("Token: ");
      Serial.println(strDeviceID);
    }
  } else {
    Serial.println("WARNING: Device not found, state not updated");
}
}


//This function handles the updating of the power value of a device based on a received message.
void handleDevicePower(String deviceString) {
  //The input message must be at least 5 characters long, otherwise it is considered invalid.
  if (deviceString.length() < 5) {
    Serial.println("ERROR: Invalid message format");
    return;
  }
  //MAX_TOKENS_POWER and MAX_LEN_POWER are defined as 3 and 9 respectively.
  #define MAX_TOKENS_POWER 3
  #define MAX_LEN_POWER 9
  //A character array called 'input' is created with a size of MAX_LEN_POWER + 1.
  char input[MAX_LEN_POWER+1];
  /*The deviceString is converted to a character array and stored in 'input'.
  An array of Strings called 'tokens_power' is created with a size of MAX_TOKENS_POWER.
  An integer variable called 'token_count' is initialized to 0.*/
  deviceString.toCharArray(input, sizeof(input));
  String tokens_power[MAX_TOKENS_POWER];
  int token_count = 0;

  //A pointer called 'ptr' is created and initialized to point to the first token in the input string.
  //While 'ptr' is not NULL and token_count is less than MAX_TOKENS_POWER:
  //The current token is stored as a string in the 'tokens_power' array.
  //token_count is incremented.
  char *ptr = strtok(input, "-");
  while (ptr != NULL && token_count < MAX_TOKENS_POWER) {
    tokens_power[token_count] = String(ptr);
    token_count++;
    //'ptr' is updated to point to the next token in the input string.
    ptr = strtok(NULL, "-");
  }

  //find the index of the device
  int deviceIndex = findDeviceIndex(tokens_power[0]);
  if (deviceIndex != -1) {
    if (devices[deviceIndex].type == 'T') {
      int temperature = tokens_power[1].toInt();
      if (temperature >= 9 && temperature <= 32) {
        devices[deviceIndex].temperature = temperature;
        Serial.println("Device temperature updated: ");
        printDevice(devices[deviceIndex]);
      } else {
        Serial.println("ERROR: Invalid temperature value");
      }
    } else if (devices[deviceIndex].type == 'S' || devices[deviceIndex].type == 'L') {
      int power = tokens_power[1].toInt();
      if (power >= 0 && power <= 100) {
        devices[deviceIndex].power = power;
        Serial.println("Device power updated: ");
        printDevice(devices[deviceIndex]);
      } else {
        Serial.println("ERROR: Invalid power value");
      }
    } else if (devices[deviceIndex].type == 'C' || devices[deviceIndex].type == 'O') {
      Serial.println("ERROR: Device does not support power update");
    } else {
      Serial.println("ERROR: Invalid device type for power update");
    }
  } else {
    Serial.println("ERROR: Device not found");
  }}





// This function takes a deviceString as input and removes the device corresponding to the ID provided in the string.
void removeDevice(String deviceString) {
  #define MAX_TOKENS_REMOVE 1
  #define MAX_LEN_REMOVE 10

  char input[MAX_LEN_REMOVE+1];
  deviceString.toCharArray(input, sizeof(input));
  char tokens_remove[MAX_TOKENS_REMOVE][MAX_LEN_REMOVE+1];
  int token_count = 0;
  // The device ID is extracted from the input string by splitting it into tokens using the '-' character as a delimiter.
  char *ptr = strtok(input, "-");
  while (ptr != NULL && token_count < MAX_TOKENS_REMOVE) {
    strncpy(tokens_remove[token_count], ptr, MAX_LEN_REMOVE);
    tokens_remove[token_count][MAX_LEN_REMOVE-1] = '\0';
    token_count++;
    ptr = strtok(NULL, "-");
  }

  // find the index of the device
  int deviceIndex = findDeviceIndex(tokens_remove[0]);
  Serial.print(tokens_remove[0]);
  Serial.println(deviceIndex);
  if (deviceIndex != -1) {
    // If a device with the given ID is found, the removeDeviceAtIndex() function is called to remove the device from the devices array.
    // If a device with the given ID is not found, a warning message is printed to the serial monitor.
    removeDeviceAtIndex(deviceIndex);
    lcd.clear();
    lcd.setBacklight(BACKLIGHT_YELLOW);
    Serial.print("Device removed: ");
    Serial.println(tokens_remove[0]);
  } else {
    Serial.print("WARNING: Device not found with ID ");
    Serial.println(tokens_remove[0]);
  }
}

//Format R-DeviceID-DeviceState


// This function removes a device at the specified index from the devices array.
int removeDeviceAtIndex(int index) {
  int totalDevices = sizeof(devices) / sizeof(devices[0]);
  // It first checks if the index is valid, and if not, returns an error message and -1
  if (index >= totalDevices || index < 0) {
    Serial.println("ERROR: Invalid device index");
    return -1;
  }
  // If the index is valid, it shifts all devices after the specified index down by one,
  // and then replaces the last element in the array with an empty Device object.
  for (int i = index; i < totalDevices - 1; i++) {
    devices[i] = devices[i + 1];
  }
  devices[totalDevices - 1] = Device();
  // Finally, it prints the removed device's information and returns 0.
  printDevice(devices[totalDevices - 1]);
  return 0;
}