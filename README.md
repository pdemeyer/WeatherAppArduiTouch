# WeatherAppArduiTouch
App for showing time, date, weather and moon phases with ArduiTouch and NodeMCU-32S

# ArduiTouch ESP
ArduiTouch is a touchscreen enclosure set made for wall mounting with NodeMCU or Wemos D1 Mini boards based on ESP8266 or ESP32 platforms.

# NodeMCU-32S
We tested ArduiTouch ESP with NodeMCU-32S, and here is how it is connected internally:

![alt](https://github.com/Slaveche90/WeatherAppArduiTouch/blob/master/ConnectionDiagram.jpg?raw=true)

# Setting up Arduino IDE

In order to program NodeMCU-32S with Arduino IDE, first install ESP32 core. You have to add one URL link to the Arduino IDE preferences. Open Arduino IDE and go to File > Preferences, a new window will open. Find field called “Additional Board Manager URLs”, and copy following url:
https://dl.espressif.com/dl/package_esp32_index.json 
Then, paste URL in "Additional Board Manager URLs” field. If you already have one URL inside, just add one comma after existing URL and paste new URL after it. After this, click the OK button, and close the Arduino IDE. 
Then you have to start Arduino IDE again. Go to: Tools > Board > Boards Manager. A new window will open. Type in “esp32” the search box and install board called “esp32” by “Espressif Systems”, and you have esp32 core installed.

# External libraries

First library is for TFT LCD screen, it is called “TFT_eSPI”. Open Arduino IDE, and go to Tools > Manage Libraries. Type the name of the library in the search box and install library “TFT_eSPI” by “Bodmer”, like on the image below:

For this library you have to edit one file. Go to directory where Arduino IDE saves sketches and libraries. If you did not change that directory, it is in Documents > Arduino. To find TFT_eSPI library directory, open File Explorer, and go to:
Documents > Arduino > libraries > TFT_eSPI 

We have to edit file called “User_Setup.h”. Open it in any text editor (for example Notepad or Sublime Text). 
Find following lines of the text (at 156 line number):
```
// ###### EDIT THE PIN NUMBERS IN THE LINES FOLLOWING TO SUIT YOUR ESP32 SETUP   
// For ESP32 Dev board (only tested with ILI9341 display)
// The hardware SPI can be mapped to any pins
//#define TFT_MISO		19
//#define TFT_MOSI		23
//#define TFT_SCLK		18
//#define TFT_CS		15	// Chip select control pin
//#define TFT_DC		2	// Data Command control pin
//#define TFT_RST		4	// Reset pin (could connect to RST pin)
//#define TFT_RST		-1	// Set TFT_RST to -1 if display RESET is 
							// connected to ESP32 board RST
//#define TFT_BL		32	// LED back-light 
							// (only for ST7789 with backlight control pin)
//#define TOUCH_CS		21	// Chip select pin (T_CS) of touch screen
//#define TFT_WR		22	// Write strobe for modified Raspberry Pi TFT only
```
and change it to this:
```
#define TFT_MISO		19
#define TFT_MOSI		23
#define TFT_SCLK		18
#define TFT_CS			5   
#define TFT_DC			4   
#define TFT_RST			22  
//#define TFT_RST		-1  
//#define TFT_BL		32  
#define TOUCH_CS		14     
//#define TFT_WR		22  
```  
After this, save the file. We need to make these changes because pins of the TFT LCD screen are connected to pins of the NodeMCU-32S like in the changed text (you can see it on the connection diagram). Backlight pin is controlled separately, so we do not set-up it here.

The second library is for Real Time API called "NTPClient". Open Arduino IDE, and go to Tools > Manage Libraries. Type "NTP" in the search box and install library “NTPClient” by “Fabrice Wenberg”.

The third library is for parsing JSON data, it is called “ArduinoJson”. Open Arduino IDE, and go to Tools > Manage Libraries. Type the name of the library in the search box and install library “ArduinoJson” by “Benoit Blanchon”.

The fourth library is for Moon Phase API, called “moonPhases”. You can not find this library in the Manage Libraries. We have to download it from the GitHub. Go on this link: https://github.com/CelliesProjects/moonPhase
and download .zip file. Then, to add it to the Arduino IDE, open Arduino IDE and go to Sketch > Include Library > Add .ZIP Library… and add the downloaded .zip file.

The fifth library is for converting timestamps, called “TimeLib”. You can not find this library in the Manage Libraries. We have to download it from the GitHub. Go on this link: https://github.com/PaulStoffregen/Time 
and download .zip file. Then, to add it to the Arduino IDE, open Arduino IDE and go to Sketch > Include Library > Add .ZIP Library… and add the downloaded .zip file.

# Contorlling the screen

In order to turn ON the screen and the following lines of code at the beginning of setup() function:
```
pinMode(15, OUTPUT);    
digitalWrite(15, LOW);  // turn ON
```

You can use PWM on digital pin 15 to adjust different levels of brightness for screen.

# Code explanation

Code is explained in detail in free eBook Quick Starter Guide for ArduiTouch, and you can download it from: https://delivery.shopifyapps.com/-/13a9ce242261d0a6/d6e380af0a3968ab