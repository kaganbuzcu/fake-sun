#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <ESPAsyncTCP.h>
#include <Hash.h>
#include <FS.h>
#include <dimmable_light.h>
#include <Wire.h>  // Library for I2C communication
#include <SPI.h>   // not used here, but needed to prevent a RTClib compile error
#include "RTClib.h"
#include <LiquidCrystal_I2C.h>  // Library for LCD

#include "htmlform.h"     // form html
#include "successpage.h"  // succes page html

ESP8266WebServer server(80);  //Server on port 80
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 16, 2);
RTC_DS3231 RTC;  // Setup an instance of DS3231 naming it RTC

//SSID and Password of your WiFi router
const char* SSID = "";
const char* PASSWORD = "";

const int seeIpAdressPin = D8;
const int relayToDimmerPin = D7;
const int relayToDirectPin = D4;
const int syncPin = D5;
const int thyristorPin = D6;
DimmableLight light(thyristorPin);

const int MIN_BRIGHTNESS = 70; // It is the value to which the bulb I use reacts first. Yours may be different
const int MAX_BRIGHTNESS = 200; // The value at which the transition to maximum brightness is imperceptibly small for the area I use. Yours may be differentThe value at which the transition to maximum brightness is imperceptibly small for the area I use. Yours may be differentThe value at which the transition to maximum brightness is imperceptibly small for the area I use. Yours may be different

// SPIFFS file read func
String readFile(fs::FS& fs, const char* path) {
  File file = fs.open(path, "r");
  if (!file || file.isDirectory()) {
    Serial.println("- empty file or failed to open file");
    return String();
  }
  String fileContent;
  while (file.available()) {
    fileContent += String((char)file.read());
  }
  file.close();
  return fileContent;
}

// SPIFFS file write func
void writeFile(fs::FS& fs, const char* path, const char* message) {
  Serial.printf("Writing file: %s\r\n", path);
  File file = fs.open(path, "w");
  if (!file) {
    Serial.println("- failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("- file written");
  } else {
    Serial.println("- write failed");
  }
  file.close();
}

// 404 page
void handleNotFound() {
  server.sendHeader("Location", "/", true);
  server.send(302, "text/plane", "");
}
// main page
void handleRoot() {
  String s = MAIN_page;
  server.send(200, "text/html", s);
}
// action page
void handleForm() {
  if (server.hasArg("sunriseTime") && !server.arg("sunriseTime").isEmpty()) {
    writeFile(SPIFFS, "/sunriseTime.txt", server.arg("sunriseTime").c_str());
  }
  if (server.hasArg("sunsetTime") && !server.arg("sunsetTime").isEmpty()) {
    writeFile(SPIFFS, "/sunsetTime.txt", server.arg("sunsetTime").c_str());
  }
  if (server.hasArg("directlyOpen") && server.arg("directlyOpen").toInt() == 0) {
    turnOn();
  }
  if (server.hasArg("directlyClose") && server.arg("directlyClose").toInt() == 1) {
    turnOff();
  }
  String s = SUCCESS_page;
  server.send(200, "text/html", s);
}
void initialize_www() {
  server.on("/", handleRoot);
  server.on("/action.html", handleForm);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}
void initialize_dimmer() {
  Serial.print("Initializing DimmableLight library... ");
  DimmableLight::setSyncPin(syncPin);
  // VERY IMPORTANT: Call this method to activate the library
  DimmableLight::begin();
  Serial.println("DimmableLight Done!");
}
// See the ip address on lcd with interupt for the connection post
void ICACHE_RAM_ATTR printIpAdress() {
  Serial.println("Ip Adress is Printing");
  if (WiFi.status() == WL_CONNECTED) {
    clearLCDLine(0);
    lcd.setCursor(0, 0);
    lcd.print(WiFi.localIP());
  } else {
    clearLCDLine(0);
    lcd.setCursor(0, 0);
    lcd.print("Wifi bagli degil"); // wifi is not connected
  }
}
void setup(void) {
  Serial.begin(115200);
  while (!Serial)
    ;
  // relay pins setup
  pinMode(relayToDimmerPin, OUTPUT);
  pinMode(relayToDirectPin, OUTPUT);
  digitalWrite(relayToDimmerPin, HIGH);
  digitalWrite(relayToDirectPin, HIGH);
  // see ip address interrupt setup
  pinMode(seeIpAdressPin, INPUT);

  if (!SPIFFS.begin()) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  Wire.begin();
  RTC.begin();
  // RTC.adjust(DateTime(2023, 12, 16, 17, 1, 30)); // to set the time

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Defne Dogal"); // project title

  initialize_dimmer();
  WiFi.begin(SSID, PASSWORD);
  initialize_www();
  attachInterrupt(digitalPinToInterrupt(seeIpAdressPin), printIpAdress, RISING);
}
void loop(void) {
  server.handleClient();  //Handle client requests
  // get current time
  DateTime now = RTC.now();
  // get previously set times 
  String sunriseTime = readFile(SPIFFS, "/sunriseTime.txt");
  String sunsetTime = readFile(SPIFFS, "/sunsetTime.txt");

  int sunrise_hour = sunriseTime.substring(0, 2).toInt();
  int sunrise_minute = sunriseTime.substring(3, 5).toInt();
  int sunset_hour = sunsetTime.substring(0, 2).toInt();
  int sunset_minute = sunsetTime.substring(3, 5).toInt();

  // print sunrise and sunset time on lcd
  printTimes(sunriseTime.substring(0, 2), sunriseTime.substring(3, 5), sunsetTime.substring(0, 2), sunsetTime.substring(3, 5));

  if (now.hour() == sunrise_hour && now.minute() == sunrise_minute) {
    riseTheSun();
  }

  if (now.hour() == sunset_hour && now.minute() == sunset_minute) {
    setTheSun();
  }

  delay(1000);
}

// print sunrise and sunset time on lcd
void printTimes(String sunrise_hour, String sunrise_minute, String sunset_hour, String sunset_minute) {
  clearLCDLine(1);
  lcd.setCursor(0, 1);
  lcd.print("D:");
  lcd.print(sunrise_hour);
  lcd.print(":");
  lcd.print(sunrise_minute);
  lcd.print(" B:");
  lcd.print(sunset_hour);
  lcd.print(":");
  lcd.print(sunset_minute);
}

void riseTheSun() {
  DateTime now = RTC.now(); // get current time
  Serial.println("Doguyor");
  digitalWrite(relayToDimmerPin, LOW); // open dimmer input relay
  const int currentMinute = now.minute();
  int brightness = MIN_BRIGHTNESS;
  int voltage = 0; // estimated voltage value to print on lcd
  while (true) {
    Serial.println(brightness);
    clearLCDLine(0);
    lcd.setCursor(0, 0);
    lcd.print("Doguyor - ");
    light.setBrightness(brightness); // set dimmer value
    brightness += 1;
    voltage = map(brightness, MIN_BRIGHTNESS, MAX_BRIGHTNESS, MIN_BRIGHTNESS, 220);
    lcd.setCursor(10, 0);
    lcd.print(String(voltage) + String("V"));

    if (brightness > MAX_BRIGHTNESS) {
      break;
    }
    // Configuration that slows first few value increments. I needed this because the first value increases overreacted to the bulb I was using. 
    // Sunrise time is approximately 18 minutes.
    if (brightness < 30 + MIN_BRIGHTNESS) {
      delay(27000);
    } else if (brightness < 60 + MIN_BRIGHTNESS) {
      delay(5000);
    } else {
      delay(3000);
    }
  }
  turnOn();
}
void turnOn() {
  // cut off the dimmer module
  digitalWrite(relayToDimmerPin, HIGH);
  delay(100);
  // open the relay
  digitalWrite(relayToDirectPin, LOW);
  clearLCDLine(0);
  delay(100);
  lcd.setCursor(0, 0);
  lcd.print("Isik Acik");
}
void setTheSun() {
  Serial.println("Batiyor...");
  digitalWrite(relayToDirectPin, HIGH);
  delay(100);
  digitalWrite(relayToDimmerPin, LOW);
  int brightness = MAX_BRIGHTNESS;
  int voltage = 0;
  while (true) {
    Serial.println(brightness);
    clearLCDLine(0);
    lcd.setCursor(0, 0);
    lcd.print("Batiyor - ");
    // set dimmer value
    light.setBrightness(brightness);
    brightness -= 1;
    voltage = map(brightness, MIN_BRIGHTNESS, MAX_BRIGHTNESS, 0, 220);
    lcd.setCursor(10, 0);
    lcd.print(String(voltage) + String("V"));
    if (brightness < MIN_BRIGHTNESS) {
      break;
    }
    // Configuration that slows down the last few decrements. I needed this because the recent dips were overreacting to the bulb I was using.
    // Sunset time is approximately 18 minutes.
    if (brightness < 30 + MIN_BRIGHTNESS) {
      delay(27000);
    } else if (brightness < 60 + MIN_BRIGHTNESS) {
      delay(5000);
    } else {
      delay(3000);
    }
  }
  turnOff();
}
void turnOff() {
  // cut off the relay
  digitalWrite(relayToDirectPin, HIGH);
  delay(100);
  // cut off the dimmer module
  digitalWrite(relayToDimmerPin, HIGH);
  clearLCDLine(0);
  delay(100);
  lcd.setCursor(0, 0);
  lcd.print("Isik Kapali");
}

// clear lcd
void clearLCDLine(int line) {
  lcd.setCursor(0, line);
  for (int n = 0; n < 20; n++)  // 20 indicates symbols in line. For 2x16 LCD write - 16
  {
    lcd.print(" ");
  }
}