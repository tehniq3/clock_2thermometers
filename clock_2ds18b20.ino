/*  Clock - Thermometer with Arduino
 *  More info: http://www.ardumotive.com/
 *  http://www.ardumotive.com/digitalclockther-en.html
 *  Dev: Michalis Vasilakis Data: 19/11/2016 Ver: 1.0
 *  change by Nicu FLORICA (niq_ro) for dual thermometer
 *  http://www.arduinotehniq.com/
 *  28.10.2018
 *  
 *  Display 16x2:         Setup:
 *  +----------------+  +----------------+
 *  |HH:MM DD/MM/YYYY|  |    >HH :>MM    |
 *  |23.5*C  Tur:74*C|  |>DD />MM />YYYY |
 *  +----------------+  +----------------+
 */

//Libraries
#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>

//Init libraries objects
RTC_DS1307 rtc;
LiquidCrystal_I2C lcd(0x3F, 16, 2);

// DS18B20 sensors
#define ONE_WIRE_BUS 2
#define TEMPERATURE_PRECISION1 11
#define TEMPERATURE_PRECISION2 11
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);
// arrays to hold device addresses
DeviceAddress Thermometer1, Thermometer2;

//Constants
char daysOfTheWeek[7][12] = {"Sunday","Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

const long interval = 6000;  // Read data from DHT every 6 sec
const int btSet = 8;
const int btUp = 9;
const int btDown = 10;

//Variables
int DD,MM,YY,H,M,S,temp2, set_state, up_state, down_state;
float temp1;
int btnCount = 0;
unsigned long previousMillis = 0;
unsigned long currentMillis; 
String sDD;
String sMM;
String sYY;
String sH;
String sM;
String sS;
boolean backlightON = true;
boolean setupScreen = false;


void setup () {
  Serial.begin(9600);
  pinMode(btSet, INPUT_PULLUP);
  pinMode(btUp, INPUT_PULLUP);
  pinMode(btDown, INPUT_PULLUP);
  lcd.begin();   // init the LCD 
  lcd.backlight();   // Turn on the backligt (try lcd.noBacklight() to turn it off)
  lcd.clear();
  sensors.begin();  // initialize sensors DS18B20
  
    // locate devices on the bus
  Serial.print("Locating devices...");
  Serial.print("Found ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" devices.");

  // report parasite power requirements
  Serial.print("Parasite power is: ");
  if (sensors.isParasitePowerMode()) Serial.println("ON");
  else Serial.println("OFF");

  // Search for devices on the bus and assign based on an index. Ideally,
  // you would do this to initially discover addresses on the bus and then
  // use those addresses and manually assign them (see above) once you know
  // the devices on your bus (and assuming they don't change).
  //
  // method 1: by index
  if (!sensors.getAddress(Thermometer1, 0)) Serial.println("Unable to find address for Device 0");
  if (!sensors.getAddress(Thermometer2, 1)) Serial.println("Unable to find address for Device 1");

  // method 2: search()
  // search() looks for the next device. Returns 1 if a new address has been
  // returned. A zero might mean that the bus is shorted, there are no devices,
  // or you have already retrieved all of them. It might be a good idea to
  // check the CRC to make sure you didn't get garbage. The order is
  // deterministic. You will always get the same devices in the same order
  //
  // Must be called before search()
  //oneWire.reset_search();
  // assigns the first address found to Thermometer2
  //if (!oneWire.search(Thermometer2)) Serial.println("Unable to find address for Thermometer2");
  // assigns the seconds address found to Thermometer1
  //if (!oneWire.search(Thermometer1)) Serial.println("Unable to find address for Thermometer1");

  // show the addresses we found on the bus
  Serial.print("Device 0 Address: ");
  printAddress(Thermometer1);
  Serial.println();

  Serial.print("Device 1 Address: ");
  printAddress(Thermometer2);
  Serial.println();
  
 // set the resolutions  per every device
  sensors.setResolution(Thermometer1, TEMPERATURE_PRECISION1);
  sensors.setResolution(Thermometer2, TEMPERATURE_PRECISION2);

  // set the resolution to 9 bit per device
  Serial.print("Device 0 Resolution: ");
  Serial.print(sensors.getResolution(Thermometer1), DEC);
  Serial.println();

  Serial.print("Device 1 Resolution: ");
  Serial.print(sensors.getResolution(Thermometer2), DEC);
  Serial.println();
}

void loop () {
  currentMillis = millis();
  readBtns();
  getTempTemp();    
  getTimeDate();
  if (!setupScreen){
    lcdPrint();
  }
  else{
    lcdSetup();
  }
}
//Read buttons
void readBtns(){
  set_state = digitalRead(btSet);
  up_state = digitalRead(btUp);
  down_state = digitalRead(btDown);
  //Turn backlight on/off by pressing the down button
  if (down_state==LOW && btnCount==0){
    if (backlightON){
      lcd.noBacklight();
      backlightON = false;
    }
    else{
      lcd.backlight();
      backlightON = true;
    }
    delay(500);
  }
  if (set_state==LOW){
    if(btnCount<5){
      btnCount++;
      setupScreen = true;
        if(btnCount==1){
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("------SET------");
          lcd.setCursor(0,1);
          lcd.print("-TIME and DATE-");
          delay(2000);
          lcd.clear();
        }
    }
    else{
      lcd.clear();
      rtc.adjust(DateTime(YY, MM, DD, H, M, 0));
      lcd.print("Saving....");
      delay(2000);
      lcd.clear();
      setupScreen = false;
      btnCount=0;
    }
    delay(500);
  }
}
//Read temperature and humidity every 6 seconds from DHT sensor
void getTempTemp(){
  if (currentMillis - previousMillis >= interval) {
  // call sensors.requestTemperatures() to issue a global temperature
  // request to all devices on the bus

  sensors.requestTemperatures();
  temp1 = sensors.getTempC(Thermometer1);
  temp2 = sensors.getTempC(Thermometer2);
  /*
  Serial.print("Requesting temperatures...");
  sensors.requestTemperatures();
  Serial.println("DONE");
  temp1 = sensors.getTempC(Thermometer1);
  Serial.print("Temp ext C: "); 
  temp2 = sensors.getTempC(Thermometer2);
  Serial.print("Temp int C: ");
  */
  Serial.print(temp1); 
  Serial.print("gr.C  --- ");
  Serial.print(temp2);
    Serial.println("gr.C-----");
    previousMillis = currentMillis;    
  }
}

//Read time and date from rtc ic
void getTimeDate(){
  if (!setupScreen){
    DateTime now = rtc.now();
    DD = now.day();
    MM = now.month();
    YY = now.year();
    H = now.hour();
    M = now.minute();
    S = now.second();
  }
  //Make some fixes...
  if (DD<10){ sDD = '0' + String(DD); } else { sDD = DD; }
  if (MM<10){ sMM = '0' + String(MM); } else { sMM = MM; }
  sYY=YY;
  if (H<10){ sH = '0' + String(H); } else { sH = H; }
  if (M<10){ sM = '0' + String(M); } else { sM = M; }
  if (S<10){ sS = '0' + String(S); } else { sS = S; }
}
//Print values to the display
void lcdPrint(){
  lcd.setCursor(0,0); //First row
  lcd.print(sH);
  lcd.print(":");
  lcd.print(sM);
  lcd.print(" ");
  lcd.print(sDD);
  lcd.print("/");
  lcd.print(sMM);
  lcd.print("/");
  lcd.print(sYY);
  lcd.setCursor(0,1); //Second row
//  lcd.print("In:");
  lcd.print(temp1);
  lcd.write(0b11011111);  // simbol de grad
  lcd.print("C");
  lcd.setCursor(8,1); //10 cell of second row
  lcd.print("Tur:");
  lcd.print(temp2);
  lcd.write(0b11011111);  // simbol de grad
  lcd.print("C");
}
//Setup screen
void lcdSetup(){
  if (btnCount==1){
    lcd.setCursor(4,0);
    lcd.print(">"); 
    if (up_state == LOW){
      if (H<23){
        H++;
      }
      else {
        H=0;
      }
      delay(500);
    }
    if (down_state == LOW){
      if (H>0){
        H--;
      }
      else {
        H=23;
      }
      delay(500);
    }
  }
  else if (btnCount==2){
    lcd.setCursor(4,0);
    lcd.print(" ");
    lcd.setCursor(9,0);
    lcd.print(">");
    if (up_state == LOW){
      if (M<59){
        M++;
      }
      else {
        M=0;
      }
      delay(500);
    }
    if (down_state == LOW){
      if (M>0){
        M--;
      }
      else {
        M=59;
      }
      delay(500);
    }
  }
  else if (btnCount==3){
    lcd.setCursor(9,0);
    lcd.print(" ");
    lcd.setCursor(0,1);
    lcd.print(">");
    if (up_state == LOW){
      if (DD<31){
        DD++;
      }
      else {
        DD=1;
      }
      delay(500);
    }
    if (down_state == LOW){
      if (DD>1){
        DD--;
      }
      else {
        DD=31;
      }
      delay(500);
    }
  }
  else if (btnCount==4){
    lcd.setCursor(0,1);
    lcd.print(" ");
    lcd.setCursor(5,1);
    lcd.print(">");
    if (up_state == LOW){
      if (MM<12){
        MM++;
      }
      else {
        MM=1;
      }
      delay(500);
    }
    if (down_state == LOW){
      if (MM>1){
        MM--;
      }
      else {
        MM=12;
      }
      delay(500);
    }
  }
  else if (btnCount==5){
    lcd.setCursor(5,1);
    lcd.print(" ");
    lcd.setCursor(10,1);
    lcd.print(">");
    if (up_state == LOW){
      if (YY<2999){
        YY++;
      }
      else {
        YY=2000;
      }
      delay(500);
    }
    if (down_state == LOW){
      if (YY>2000){
        YY--;
      }
      else {
        YY=2999;
      }
      delay(500);
    }
  }
  lcd.setCursor(5,0);
  lcd.print(sH);
  lcd.setCursor(8,0);
  lcd.print(":");
  lcd.setCursor(10,0);
  lcd.print(sM);
  lcd.setCursor(1,1);
  lcd.print(sDD);
  lcd.setCursor(4,1);
  lcd.print("/");
  lcd.setCursor(6,1);
  lcd.print(sMM);
  lcd.setCursor(9,1);
  lcd.print("/");
  lcd.setCursor(11,1);
  lcd.print(sYY);
}

// function to print a device address
void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    // zero pad the address if necessary
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}

// function to print the temperature for a device
void printTemperature(DeviceAddress deviceAddress)
{
  float tempC = sensors.getTempC(deviceAddress);
  Serial.print("Temp C: ");
  Serial.print(tempC);
  Serial.print(" Temp F: ");
  Serial.print(DallasTemperature::toFahrenheit(tempC));
}

// function to print a device's resolution
void printResolution(DeviceAddress deviceAddress)
{
  Serial.print("Resolution: ");
  Serial.print(sensors.getResolution(deviceAddress));
  Serial.println();
}

// main function to print information about a device
void printData(DeviceAddress deviceAddress)
{
  Serial.print("Device Address: ");
  printAddress(deviceAddress);
  Serial.print(" ");
  printTemperature(deviceAddress);
  Serial.println();
}
