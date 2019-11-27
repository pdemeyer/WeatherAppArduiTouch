#include "SPI.h"
#include "TFT_eSPI.h"

#include <ArduinoJson.h>

#include <WiFi.h>
#include <WiFiClient.h>

#include <TimeLib.h>

#include <NTPClient.h>
#include <WiFiUdp.h>

#include <moonPhase.h>

#include "xbm.h"
#include "moonPhs.h"

unsigned long second_interval = 1000;   // one second in miliseconds
unsigned long weather_interval = 3600;  // read weather data every 3600 seconds (every hour)
unsigned long counter = 0;    // for counting the seconds
unsigned long time_T;         // current time from millis()
unsigned long last_update_second = 0;

moonPhase myMoonPhase;
moonData_t currentMoon;

// Replace with your SSID and password details
char ssid[] = "wg-19o03522_13";
char pass[] = "92m1.4-53s0_A";

String text; // for JSON parsing

WiFiClient client;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Open Weather Map API server name
const char server[] = "api.openweathermap.org";
String nameOfCity = "Novi Sad,RS";
String apiKey = "49e9034244891f66bcd6e790c702155f";

int jsonend = 0;
boolean startJson = false;
int status = WL_IDLE_STATUS;

#define JSON_BUFF_DIMENSION 2500

TFT_eSPI dsply = TFT_eSPI();

void printText(int x, int y, String text, uint8_t textSize = 1, uint8_t textAllign = 1, uint8_t lineLength = 239) {
  /*  This function is used for displaying text on the screen
   *  Arguments:
   *    - X           position of the cursor
   *    - Y           position of the cursor
   *    - text        the actual text that will be displayed
   *    - textSize    text size can be one of these values 1, 2, 3, 4, 5
   *    - textAllign  text allign can be 1 - left align, 2 - center and 3 - right align
   *    - lineLenght  this should be used for line lenght of text, but does not works as shoud - TODO
   *    
   *  Returns:
   *  nothing
   */
  
  uint8_t newTextSize = textSize;
  uint8_t real_x = 0;
  uint32_t stringLength = text.length();
  uint8_t characters = stringLength * 5 * newTextSize + stringLength * newTextSize;

  while ((characters + 10) > lineLength) {
    // make text smaller if it is longer than line length
    // all text in this app is not (and it should not be) longer than line length
    newTextSize = newTextSize - 1;
    characters = stringLength * 5 * newTextSize + stringLength * newTextSize;
  }
  dsply.setTextSize(newTextSize);

  if ((stringLength > 16) && (newTextSize > 2)) {
    // there is an error with text that is 17 characters long with size of 2
    // so this IF statement is explicitly for that error, to make text size smaller
    newTextSize = newTextSize - 1;
    characters = stringLength * 5 * newTextSize + stringLength * newTextSize;
  }
  dsply.setTextSize(newTextSize);

  if (characters + 10 < lineLength) {
    if (textAllign == 1) { // left
      dsply.setCursor(x, y);
      dsply.println(text);
    }
    else if (textAllign == 2) { // centered
      if (textSize == 1) { // letter length = 5
        real_x = x + int((lineLength - characters) / 2);
        dsply.setCursor(real_x, y);
        dsply.println(text);
      }
      else if (textSize == 2) { // letter length = 10
        real_x = x + int((lineLength - characters) / 2);
        dsply.setCursor(real_x, y);
        dsply.println(text);
      }
      else if (textSize == 3) { // letter length = 15
        real_x = x + int((lineLength - characters) / 2);
        dsply.setCursor(real_x, y);
        dsply.println(text);
      }
      else if (textSize == 4) { // letter length = 20
        real_x = x + int((lineLength - characters) / 2);
        dsply.setCursor(real_x, y);
        dsply.println(text);
      }
      else if (textSize == 5) { // letter length = 25
        real_x = x + int((lineLength - characters) / 2);
        dsply.setCursor(real_x, y);
        dsply.println(text);
      }
      else {
        dsply.setTextSize(1);
        dsply.setCursor(x, y);
        dsply.println("ERROR! Text size is from 1 to 5!");
      }
    }
    else if (textAllign == 3) { // right
      if (textSize == 1) { // letter length = 5
        real_x = x + lineLength - characters;
        dsply.setCursor(real_x, y);
        dsply.println(text);
      }
      else if (textSize == 2) { // letter length = 10
        real_x = x + lineLength - characters;
        dsply.setCursor(real_x, y);
        dsply.println(text);
      }
      else if (textSize == 3) { // letter length = 15
        real_x = x + lineLength - characters;
        dsply.setCursor(real_x, y);
        dsply.println(text);
      }
      else if (textSize == 4) { // letter length = 20
        real_x = x + lineLength - characters;
        dsply.setCursor(real_x, y);
        dsply.println(text);
      }
      else if (textSize == 5) { // letter lenght = 25
        real_x = x + lineLength - characters;
        dsply.setCursor(real_x, y);
        dsply.println(text);
      }
      else {
        dsply.setTextSize(1);
        dsply.setCursor(x, y);
        dsply.println("ERROR! Text size is from 1 to 5!");
      }
    }
    else {
      dsply.setTextSize(1);
      dsply.setCursor(x, y);
      dsply.println("ERROR! TextAlign is 0, 1 and 2!");
    }
  }
  else {
    dsply.setCursor(x, y);
    dsply.println(text);
  }
}

void initializeLCD() {
  /*  This function is used to turn ON the LCD and to initialize dsply object
   *  Arguments:
   *  none
   *  
   *  Returns:
   *  nothing
   */
   
  // turn ON the dispaly
  pinMode(15, OUTPUT);    // Backlight pin of the display is connecte to this pin of ESP32
  digitalWrite(15, LOW);  // we have to drive this pin LOW in order to turn ON the display
  delay(1);               // delay for 1ms

  dsply.init();
  dsply.fillScreen(TFT_BLACK);    //  fill the screen with black color
  dsply.setTextColor(TFT_GREEN);  //  set the text color
  
  //dsply.setFreeFont(FSS9);      //  you can use some other font, 
                                  //  but you have to change positions of every element, 
                                  //  because for different font there are different text sizes
}

void printWiFiStatus() {
  /*  This function is used to print the status of wifi connection
   *  Arguments:
   *  none
   *  
   *  Returns:
   *  nothing
   */
   
  // print the SSID of the network you're attached to:
  printText(3, 33, "SSID: ", 1, 1); delay(10);
  printText(35, 33, String(WiFi.SSID()), 1, 1); delay(10);

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  printText(3, 53, "IP Address: ", 1, 1); delay(10);
  
  String cont = "";
  for (int i = 0; i < 4; i++) {
    cont += ip[i];
    if (i == 3) {
      break;
    }
    cont += ".";
  }
  printText(70, 53, cont, 1, 1); delay(10);

  // print the received signal strength:
  printText(3, 73, "Signal strength (RSSI):", 1, 1); delay(10);
  printText(150, 73, String(WiFi.RSSI()), 1, 1); delay(10);
  printText(170, 73, "dBm", 1, 1); delay(10);
}

void makehttpRequest() {
  /*  This function is used to make http request to request data from OWM API
   *  Arguments:
   *  none
   *  
   *  Returns:
   *  nothing
   */
  
  dsply.fillScreen(TFT_BLACK);
  // close any connection before send a new request to
  // allow client make connection to server
  client.stop();
  printText(3, 83, "Making a request for OWM...", 1, 1); delay(10);

  // if there is a successful connection:
  if (client.connect(server, 80)) {
    // send the HTTP PUT request:
    String t1 = "GET /data/2.5/forecast?q=" + nameOfCity + "&APPID=" + apiKey + "&mode=json&units=metric&cnt=2 HTTP/1.1";
    String t2 = "Host: api.openweathermap.org"; 
    String t3 = "User-Agent: ArduinoWiFi/1.1"; 
    String t4 = "Connection: close";
    client.println(t1); 
    client.println(t2); 
    client.println(t3); 
    client.println(t4); 
    client.println();
    
    printText(3, 93, t1, 1, 1); 
    printText(3, 133, t2, 1, 1);
    printText(3, 143, t3, 1, 1); 
    printText(3, 153, t4, 1, 1); delay(10);

    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 5000) {
        printText(3, 163, ">>> Client Timeout !", 1, 1);
        client.stop();
        return;
      }
    }

    char c = 0;
    while (client.available()) {
      c = client.read();
      printText(3, 163, "Client Readout!");
      /* since json contains equal number of open and
       * close curly brackets, this means we can determine
       * when a json is completely received  by counting
       * the open and close occurences,
       */ 
      Serial.print(c); // for debugging
      if (c == '{') {
        startJson = true; // set startJson true to indicate json message has started
        jsonend++;
      }
      if (c == '}') {
        jsonend--;
      }
      if (startJson == true) {
        text += c;
      }
      // if jsonend = 0 then we have received equal number of curly braces
      if (jsonend == 0 && startJson == true) {
        parseJson(text.c_str());  // parse c string text in parseJson function
        text = "";                // clear text string for the next time
        startJson = false;        // set startJson to false to indicate that a new message has not yet started
      }
    }
  }
  else {
    printText(3, 163, "Connection failed!", 1, 1); // if connction was not made
    return;
  }
  delay(10);
}

void parseJson(const char * jsonString) {
  /*  This function is used to parse JSON data
   *  Arguments:
   *  jsonString      constant char array containing the JSON data from OWM API
   *  
   *  Returns:
   *  nothing
   */
  
  //StaticJsonDocument<4000> doc;
  const size_t bufferSize = 2 * JSON_ARRAY_SIZE(1) + JSON_ARRAY_SIZE(2) + 4 * JSON_OBJECT_SIZE(1) + 3 * JSON_OBJECT_SIZE(2) + 3 * JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(5) + 2 * JSON_OBJECT_SIZE(7) + 2 * JSON_OBJECT_SIZE(8) + 720;
  DynamicJsonDocument doc(bufferSize);

  // FIND FIELDS IN JSON TREE
  DeserializationError error = deserializeJson(doc, jsonString);
  if (error) {
    printText(3, 163, "deserializeJson() failed with code ", 1, 1);
    printText(3, 173, error.c_str(), 1, 1);
    return;
  }

  // variables for JSON data
  JsonArray list = doc["list"];
  JsonObject current = list[0];

  String city = doc["city"]["name"];
  uint32_t timezone = doc["city"]["timezone"];
  float temp = current["main"]["temp"];
  float humidity = current["main"]["humidity"];
  String weather = current["weather"][0]["main"];
  String weatherDescription = current["weather"][0]["description"];
  uint32_t sunrise = doc["city"]["sunrise"];
  uint32_t sunset = doc["city"]["sunset"];
  String weatherCode = current["weather"][0]["icon"];

  printText(3, 183, weatherCode);         // For debugging
  printText(3, 193, String(timezone));    // For debugging
  printText(3, 203, String(temp));        // For debugging
  delay(100);

  // get the time and date data from the NTP server
  intializeTime(timezone);  // we need to pass timezone offset here
  uint32_t dateTime = readTimeDate();

  dsply.fillScreen(TFT_BLACK);

  // Call the function for displaying the data on the screen
  showPage(dateTime, city, temp, humidity, weather, weatherDescription, sunrise + timezone, sunset + timezone, weatherCode);
}

void showWeatherIcon(String weather) {
  /*  This function is used for displaying weather icons
   *  Arguments
   *  - weather   This is code name for the image of the specific weather used in OWM API
   *  
   *  Returns:
   *  nothing
   */
   
  dsply.setSwapBytes(true); // need to swap bytes because image is inverted
  if (weather == "11d" || weather == "11n") {
    dsply.pushImage(20, 118, image_width, image_width, thunderstorm);
  }
  else if (weather == "01d") {
    dsply.pushImage(20, 118, image_width, image_width, clearSkyDay);
  }
  else if (weather == "01n") {
    dsply.pushImage(20, 118, image_width, image_width, clearSkyNight);
  }
  else if (weather == "02d") {
    dsply.pushImage(20, 118, image_width, image_width, fewCloudsDay);
  }
  else if (weather == "02n") {
    dsply.pushImage(20, 118, image_width, image_width, fewCloudsNight);
  }
  else if (weather == "03d" || weather == "03n") {
    dsply.pushImage(20, 118, image_width, image_width, scatteredClouds);
  }
  else if (weather == "04d" || weather == "04n") {
    dsply.pushImage(20, 118, image_width, image_width, brokenClouds);
  }
  else if (weather == "09d" || weather == "09n") {
    dsply.pushImage(20, 118, image_width, image_width, showerRain);
  }
  else if (weather == "10d") {
    dsply.pushImage(20, 118, image_width, image_width, rainDay);
  }
  else if (weather == "10n") {
    dsply.pushImage(20, 118, image_width, image_width, rainNight);
  }
  else if (weather == "13d" || weather == "13n") {
    dsply.pushImage(20, 118, image_width, image_width, snow);
  }
  else if (weather == "50d" || weather == "50n") {
    dsply.pushImage(20, 118, image_width, image_width, mist);
  }
  dsply.setSwapBytes(false);
}

void printTime(uint16_t x, uint16_t y, uint32_t dateTime, uint8_t textSize, uint8_t textAlign, uint8_t lineLenght = 239) {
  /*  This function is used to print Time from epoch timestamp
   *  Arguments:
   *  - X           positon of the cursor
   *  - Y           positon of the cursor
   *  - dateTime    epoch timestamp (unix timestamp)
   *  - textSize    text size can be one of these values 1, 2, 3, 4, 5
   *  - textAllign  text allign can be 1 - left align, 2 - center and 3 - right align
   *  - lineLenght  this should be used for line lenght of text, but does not works as shoud - TODO  
   *  
   *  Retruns:
   *  nothing
   */
   
  time_t t = dateTime; // dateTime is epoch timestamp
  String twoDots = ":";
  String mytime = String(checkZero(hour(t))) + twoDots + String(checkZero(minute(t))) + twoDots + String(checkZero(second(t)));
  printText(x, y, mytime, textSize, textAlign, lineLenght);
}

void printDayAndDate(uint16_t x, uint16_t y, uint32_t dateTime, uint8_t textSize, uint8_t textAlign, uint8_t lineLenght = 239) {
  /*  This function is used to print date from epoch timestamp
   *  Arguments:
   *  - X           positon of the cursor
   *  - Y           positon of the cursor
   *  - dateTime    epoch timestamp (unix timestamp)
   *  - textSize    text size can be one of these values 1, 2, 3, 4, 5
   *  - textAllign  text allign can be 1 - left align, 2 - center and 3 - right align
   *  - lineLenght  this should be used for line lenght of text, but does not works as shoud - TODO  
   *  
   *  Retruns:
   *  nothing
   */
  
  time_t t = dateTime; // dateTime is epoch timestamp
  String daysOfWeek[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
  String monthNames[12] = {"January ", "February ", "March ", "April ", "May ", "June ", "Jully ", "August ", "September ", "October ", "November ", "Dececember "};
  
  String dayName = daysOfWeek[weekday(t) - 1];
  String monthName = monthNames[month(t) - 1];

  printText(x, y + 10, dayName, textSize - 1, textAlign);
  String date = String(checkZero(day(t))) + ". " + monthName + String(year(t));
  printText(x, y + 20, date, textSize, textAlign);
}

String checkZero(uint32_t number) {
  /*  This function is used to check if the unsigned integer number is a single digit, and to add zero before single digit number
   *  Argument:
   *  - number      unsigned ingeger number
   *  
   *  Returns:
   *  String        String made from unsigned integer number
   */
   
  char buf[10];
  if (number < 10) {
    return "0" + String(number);
  }
  else {
    return String(number);
  }
}

String checkZeroTemp(float number) {
  /*  This function is used to check if the signed float number is a single digit
   *  This function convert signed float number (number from OWM API) into integer and adds zero before single digit number
   *  Argument:
   *  - number      signed float number
   *  
   *  Returns:
   *  - String        String made from unsigned integer number
   */
   
  int num = int(number);
  if (num >= 0 && num < 10) {
    return "0" + String(num);
  }
  else if (num > -10 && num < 0) {
    return "-0" + String(0 - num);
  }
  else {
    return String(num);
  }
}

void intializeTime(uint32_t timezone) {
  /*  This function is used to make the connection with NTP API and initialize the timezone
   *  Arguments:
   *  - timezone    unsigned integer number representing seconds offset of specific timezone from UTC time
   */
   
  timeClient.begin();
  timeClient.setTimeOffset(timezone);
}

uint32_t readTimeDate() {
  /*  This function is used to read the time form NTP API
   *  Arguments:
   *  none
   *  
   *  Returns:
   *  epoch timestamp from NTP API
   */
   
  while (!timeClient.update()) {
    timeClient.forceUpdate();
  }
  return timeClient.getEpochTime();
}

void updateTime() {
  /*  This function is used to dusplay time on the screen
   *  Arguments:
   *  none
   *  
   *  Returns:
   *  none
   */
   
  dsply.fillRect(0, 71, 240, 30, TFT_BLACK); 
  printTime(0, 72, readTimeDate(), 4, 2);
}

void showPage(uint32_t dateTime, String city, float temp, float humidity, String weather, String weatherDescription, uint32_t sunrise, uint32_t sunset, String weatherCode) {
  /*  This function is used to display data on the screen (APP default page)
   *  Arguments:
   *  - dateTime              epoch timestamp
   *  - city                  city name for weather data
   *  - temp                  temperature data
   *  - humidity              humidity data
   *  - weather               the name of weather from OWM API 
   *  - weatherDescription    the name of the weather condition (long version)
   *  - sunrise               epoch timestamp of sunrise timestamp
   *  - sunset                epoch timestamp of sunset timestamp
   *  - weatherCode           the code name of image used in OWM API
   *  
   *  Retruns:
   *  nothing
   */
  
  dsply.fillScreen(TFT_BLACK); // clear the screen

  printText(0, 3, city, 3, 2);  // city name

  printDayAndDate(0, 23, dateTime, 2, 2); // day name and long date

  printTime(0, 72, dateTime, 4, 2); //  time

  showWeatherIcon(weatherCode);   // weather icon

  String mytemp = checkZeroTemp(int(temp)) + "C";
  printText(130, 125, mytemp, 3, 1);  //  temperature

  String procent = "%";
  String humid = checkZero(int(humidity)) + procent;
  printText(130, 160, humid, 3, 1);   //  humidity

  //printText(100, 160, weather, 3, 1); // weather name, not used in the app
  printText(0, 200, weatherDescription, 2, 2);  //  long weather name

  printText(10, 230, "Sunrise: ", 1, 1);
  printTime(65, 230, sunrise, 1, 1);    //  sunrise time
  printText(125, 230, " Sunset: ", 1, 1);
  printTime(180, 230, sunset, 1, 1);    //  sunset time

  currentMoon = myMoonPhase.getPhase(dateTime); //  read moon data
  uint16_t angle = currentMoon.angle;
  uint8_t illumination = currentMoon.percentLit * 100;
  showMoon(angle, illumination);    // show moon icon
  String illum = "Illum: " + String(illumination) + procent;
  String age = "Age: " + String(int(angle / 29.53)) + " day(s)"; 
  printText(100, 265, "Phase of the Moon:");
  printText(100, 280, age);   //  moon age
  printText(100, 295, illum); //  moon illumination
}

void connectToWiFi() {
  /*  This function is used to connect to local WiFi network
   *  Arguments:
   *  none
   *  
   *  Retruns:
   *  nothing
   */
   
  WiFi.begin(ssid, pass);
  printText(3, 3, "connecting...", 1, 1); delay(1000);
  
  while (WiFi.status() != WL_CONNECTED) { delay(500); }
  
  printText(3, 23, "WiFi Connected", 1, 1); delay(1000);
  printWiFiStatus(); // call the function to display wifi status on the screen
}

void showMoon(uint16_t angle, uint8_t illumination) {
  /*  This function is used to diplay moon icon on the screen
   *  Arguments:
   *  - angle         age of the moon
   *  - illumination  illumination level of the moon
   *  
   *  Retruns:
   *  nothing
   */

  if (angle >= 0 && angle < 21) {
    dsply.pushImage(20, 255, moon_width, moon_width, sixteen);
  }
  else if (angle >= 21 && angle < 42) {
    dsply.pushImage(20, 255, moon_width, moon_width, one);
  }
  else if (angle >= 42 && angle < 63) {
    dsply.pushImage(20, 255, moon_width, moon_width, two);
  }
  else if (angle >= 63 && angle < 84) {
    dsply.pushImage(20, 255, moon_width, moon_width, three);
  }
  else if (angle >= 84 && angle < 105) {
    dsply.pushImage(20, 255, moon_width, moon_width, four);
  }
  else if (angle >= 105 && angle < 126) {
    dsply.pushImage(20, 255, moon_width, moon_width, five);
  }
  else if (angle >= 126 && angle < 147) {
    dsply.pushImage(20, 255, moon_width, moon_width, six);
  }
  else if (angle >= 147 && angle < 168) {
    dsply.pushImage(20, 255, moon_width, moon_width, seven);
  }
  else if (angle >= 168 && angle < 189) {
    dsply.pushImage(20, 255, moon_width, moon_width, eight);
  }
  else if (angle >= 189 && angle < 210) {
    dsply.pushImage(20, 255, moon_width, moon_width, nine);
  }
  else if (angle >= 210 && angle < 231) {
    dsply.pushImage(20, 255, moon_width, moon_width, ten);
  }
  else if (angle >= 231 && angle < 252) {
    dsply.pushImage(20, 255, moon_width, moon_width, eleven);
  }
  else if (angle >= 252 && angle < 273) {
    dsply.pushImage(20, 255, moon_width, moon_width, twelve);
  }
  else if (angle >= 273 && angle < 294) {
    dsply.pushImage(20, 255, moon_width, moon_width, thirteen);
  }
  else if (angle >= 294 && angle < 315) {
    dsply.pushImage(20, 255, moon_width, moon_width, fourteen);
  }
  else if (angle >= 315 && angle < 336) {
    dsply.pushImage(20, 255, moon_width, moon_width, fiveteen);
  }
  else if (angle >= 336) {
    dsply.pushImage(20, 255, moon_width, moon_width, sixteen);
  }
}

void setup() {
  initializeLCD();
  text.reserve(JSON_BUFF_DIMENSION);
  connectToWiFi();
  makehttpRequest();
}

void loop() {
  time_T = millis();
  if (time_T - last_update_second > second_interval) {
    // update time every second
    updateTime();
    last_update_second = time_T;
    counter++;
  }
  
  if(counter > weather_interval) {
    // update weather every hour
    makehttpRequest();
    counter = 0;    
  }
}
