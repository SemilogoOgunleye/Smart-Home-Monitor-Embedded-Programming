# Smart-Home-Monitor-Embedded-Programming
1 FSMs
I used a range of different states in the code which includes, Initialisation state,
Synchronisations state, Main program loop state, button press states and error states. In
the initialisation state, the arduino initializes its various components, such as the LCD
display and the EEPROM memory, and performs any necessary setup tasks. The arduino
then transitions to the synchronization state. For the Synchronization state, it syncs its data
with the Arduino by sending and receiving messages. The Arduino remains in its state until
it receives confirmation that the synchronization was successful, at which point it
transitions to the main program loop state. If an error occurs during this phase, the arduino
transition to an error state. For the main program loop state, the arduino displays a basic
menu on the LCD display and waits for user input. When a button is pressed, the arduino
transitions to the appropriate button press state based on the button that was pressed. For
the button pressed state, there are several button press states, one for each button on the
LCD shield. When a button is pressed, the arduino transitions to the appropriate button
press state based on the button that was pressed. In each button press state, the arduino
performs the appropriate action and then transitions back to the main program loop state.
Lastly the error state. The arduino displays an error message on the LCD display and waits
for user input. When a button is pressed, the arduino transitions back to the
synchronization state to attempt synchronization again.
2 Data structures
Constants:
BACKLIGHT_YELLOW, BACKLIGHT_GREEN, BACKLIGHT_PURPLE, and
BACKLIGHT_WHITE: integer values that represent different backlight colors for the LCD
display.
BUTTON_SELECT: integer value that represents the SELECT button on the LCD shield.
DEVICE_ADDRESS_START: integer value that represents the starting address of the device
in EEPROM.
DEVICE_SIZE: integer value that represents the size of each device in EEPROM.
MAX_DEVICES: integer value that represents the maximum number of devices that can be
stored.
Struct:
Device: a struct that contains the properties of a device (id, type, location, state, power, and
temperature).
Classes:
Adafruit_RGBLCDShield: a class that provides an interface to the RGB LCD shield.
Adafruit_MCP23017: a utility class that provides an interface to the MCP23017 I/O
expander.
LiquidCrystal: a class that provides an interface to the 16x2 LCD display.
Variables:
selectButtonPressed: a boolean variable that indicates whether the SELECT button is
currently pressed.
selectButtonReleased: a boolean variable that indicates whether the SELECT button is
currently released.
selectButtonPressedTime: an unsigned long integer variable that represents the time at
which the SELECT button was pressed.
backlightColor: an integer variable that represents the current backlight color of the LCD
display.
isSynchronized: a boolean variable that indicates whether the synchronization phase is
complete.
isDisplayCleared: a boolean variable that indicates whether the LCD display has been
cleared.
lastSyncTime: an unsigned long integer variable that represents the time at which the last
synchronization message was sent.
devices: an array of Device objects that contains the devices stored in EEPROM.
numDevices: an integer variable that represents the number of devices stored in EEPROM.
lcd: an instance of the Adafruit_RGBLCDShield class that represents the LCD shield.
The following functions update the global data structures/store:
writeDeviceToEEPROM(int deviceIndex, Device device): writes the given Device object to
EEPROM at the specified index.
readDeviceFromEEPROM(int deviceIndex): reads a Device object from EEPROM at the
specified index and returns it.
readDevicesFromEEPROM(): reads all valid Device objects from EEPROM and stores them
in the devices array.
setEEPROMFlag(bool value): writes a boolean value to a flag in EEPROM that indicates
whether the devices have been stored in EEPROM.
storeDevicesInEEPROM(): writes all devices in the devices array to EEPROM and sets the
EEPROM flag to true.
synchronize(): sends synchronization messages to the serial port and waits for a response.
Sets the isSynchronized flag to true if the response is valid. If an error occurs, displays an
error message on the LCD display.
4 Reflection
I noticed a few things that could be improved in the code. First, there is the lack of error handling
in some cases. For example, the readDeviceFromEEPROM() function assumes that there is a valid
device at the specified address, but this may not always be the case. Similarly, the
readDevicesFromEEPROM() function assumes that the EEPROM contains valid data, but this may
not be the case if the data has been corrupted or has not been written yet. Adding error handling
in these cases could improve the reliability of the system. To address these issues, I would start
by adding error handling to functions that assume the presence of valid data, such as the
readDeviceFromEEPROM() and readDevicesFromEEPROM() functions. Another change I would
make is to modify the way the data is stored in EEPROM. The current implementation uses a fixed
address for each device, which means that if one device is removed or added, the rest of the
devices' data will need to be shifted to accommodate the change. This could lead to errors and
fragmentation over time, especially if many devices are added and removed frequently. Instead, I
would using a implement linked list structure or a similar data structure to store the devices'
data in a more efficient and flexible manner. Another thing that I notice is the use of a number
100 to set the EEPROM_FLAG_ADDRESS. It would be better if this value was set in a variable with
an explicit name, as it is easier to remember and more readable. Another issue is the lack of
handling of unexpected inputs. For example, when an invalid character is received during
synchronization, the code simply displays an error message and goes back to waiting for
synchronization. Instead, it would be better to handle the error in a more user-friendly manner
or restart the synchronization process. Overall, the code is well-structured and uses clear names
for variables and functions and well commented for easier understanding.
Extension Features
1 UDCHARS
The UDCHARS aspect of the code is related to creating custom characters for the LCD
display. The code defines two byte arrays, UP_ARROW and DOWN_ARROW, which
represent the custom characters for displaying arrows pointing up and down. These byte
arrays are then passed to the lcd.createChar() function to create the custom characters
with character codes 1 and 2.
The void printDevice(Device device) function is responsible for printing device information
to the LCD display. In addition to the custom characters, it takes a Device object as input
and creates strings to represent the device state, power level, and temperature. These
strings are then printed to the LCD using the lcd.print() function.
The important variables in this code are UP_ARROW and DOWN_ARROW, which define the
custom characters for the LCD display, and stateString, powerString, and
temperatureString, which represent the device state, power level, and temperature as
strings. The important code is the lcd.createChar() function calls that create the custom
characters, and the lcd.print() function calls that print the device information to the LCD
display.
2 FREERAM
The FREERAM implementation in the code is a function called FreeRam() that returns the
amount of free memory in bytes available on the Arduino board. This function is
implemented using two global variables: __heap_start and *__brkval, which define the start
and end of the memory heap respectively.
The FreeRam() function calculates the amount of free memory by subtracting the address
of the top of the stack (which is the address of a local variable called free_memory) from
either the address of the start of the heap (__heap_start) or the address of the current heap
position (__brkval). The resulting value is the amount of free memory in bytes.
In the loop() function, the SELECT button is checked for a long press (more than one
second) and if detected, the FreeRam() function is called to display the amount of free
memory on the LCD screen. The displayed information includes the student ID number and
the amount of free SRAM.
3 EEPROM
The code includes the EEPROM library to read and write data to the EEPROM memory of
the microcontroller. The code defines the start address and size of the data to be stored in
the EEPROM using the DEVICE_ADDRESS_START and DEVICE_SIZE constants.
The code defines a struct named Device that stores information about a smart home device.
The Device struct contains several data members, including a device ID, type, location,
state, power, and temperature. The devices array is an array of Device objects that stores
information about all the smart home devices.
The code includes the readDeviceFromEEPROM() and writeDeviceToEEPROM() functions
to read and write the Device objects to the EEPROM memory. The
readDevicesFromEEPROM() function reads all the Device objects stored in the EEPROM
memory and populates the devices array.
The setEEPROMFlag() and getEEPROMFlag() functions are used to set and read a flag in the
EEPROM memory to indicate whether the data has been stored in the EEPROM memory.
The setup() function initializes the EEPROM flag to false if it is not already set. If the flag is
set, the readDevicesFromEEPROM() function is called to read the devices from the
EEPROM memory.
The storeDevicesInEEPROM() function is used to write the Device objects to the EEPROM
memory. This function is called when the smart home devices are modified, and the
changes need to be stored in the EEPROM memory.
In terms of changes to the FSM, the EEPROM functionality enables the storage of device
data in non-volatile memory. This means that the devices' data can be stored even if the
microcontroller loses power or is restarted. The FSM can now include transitions to read
and write the device data to and from the EEPROM memory. For example, the FSM can
include a state for device modification, which will transition to the
storeDevicesInEEPROM() function to write the modified device data to the EEPROM
memory. Similarly, the FSM can include a state for initializing the devices, which will
transition to the readDevicesFromEEPROM() function to read the devices from the
EEPROM memory.
