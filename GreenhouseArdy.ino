#include <DHT.h>
#include <XBee.h>
#include "U8glib.h"
#include <LiquidCrystal_I2C.h>      // using the LCD I2C Library from https://bitbucket.org/fmalpartida/new-liquidcrystal/downloads
#include <Wire.h>

//Display
U8GLIB_NHD_C12864 u8g(13, 11, 10, 9, 8);
// set the LCD address to 0x27 for a 20 chars 4 line display
// Set the pins on the I2C chip used for LCD connections:
//                    addr, en,rw,rs,d4,d5,d6,d7,bl,blpol
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address
#define BACK_LIGHT_PIN 5       // pin-5 is used to control the lcd back light

//Humidity and Temp
#define DHTPIN 2     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE);

//Xbee
XBee xbee = XBee();
ZBRxResponse rx = ZBRxResponse();
ZBRxIoSampleResponse ioSample = ZBRxIoSampleResponse();
XBeeAddress64 test = XBeeAddress64();
const int bufferSize = 140;
char bufferValue[bufferSize]; // enough space to store the string we're going to send


int CO2In = A3;
int heatRelay = 4;
int fanRelay = 3;
int humidifierRelay = 2;
int lightRelay = 8;

const int heaterLED = 8;
const int fanLED = 6;
const int humidifierLED = 5;
const int lightLED = 4;
const int motionPin = 2; // choose the input pin (for PIR sensor)

int light = 0;
float CO2 = 0;
float DHThumidity = 0;
float DHTtemp = 0;

char tempString[10];
char humidityString[10];
char lightString[10];
char CO2String[10];

void setup() {
  Serial.begin(9600);

  dht.begin();
  // Set the default voltage of the reference voltage

  analogReference(DEFAULT);

  pinMode(BACK_LIGHT_PIN, INPUT);
  pinMode(motionPin, INPUT);
  pinMode(heaterLED, OUTPUT);
  pinMode(fanRelay, OUTPUT);
  pinMode(humidifierLED, OUTPUT);
  pinMode(lightLED, OUTPUT);

  digitalWrite(BACK_LIGHT_PIN, HIGH);   //  default LCd back light is OFF
  digitalWrite(motionPin, LOW);
  digitalWrite(heaterLED, LOW);
  digitalWrite(fanLED, LOW);
  digitalWrite(humidifierLED, LOW);
  digitalWrite(lightLED, LOW);

  //screen
  lcd.begin(20, 4);
  lcd.backlight();

  u8g.setContrast(0); // Config the contrast to the best effect
  u8g.setRot180();// rotate screen, if required
  // set SPI backup if required
  //u8g.setHardwareBackup(u8g_backup_avr_spi);
  // assign default color value
  if ( u8g.getMode() == U8G_MODE_R3G3B2 ) {
    u8g.setColorIndex(255);     // white
  }
  else if ( u8g.getMode() == U8G_MODE_GRAY2BIT ) {
    u8g.setColorIndex(3);         // max intensity
  }
  else if ( u8g.getMode() == U8G_MODE_BW ) {
    u8g.setColorIndex(1);         // pixel on
  }
  else if ( u8g.getMode() == U8G_MODE_HICOLOR ) {
    u8g.setHiColorByRGB(255, 255, 255);
  }
}

void loop() {

  //Motion send immediatly (Needs Functionin)
  int greenMotion = digitalRead(motionPin);
  int pirOut = 0;
  if (greenMotion == HIGH) {
    //    digitalWrite(ledPin, HIGH);
    greenMotion = 1;
  }

  //Light
  light = analogRead(1);    //connect sensor to Analog 0
  dtostrf(light, 3, 0, lightString);
  Serial.print("Light: "); Serial.println(light); //print the value to serial

  //DHT22
  float DHThumidity = dht.readHumidity();
  dtostrf(DHThumidity, 3, 0, humidityString);
  float DHTtemp = dht.readTemperature();
  dtostrf(DHTtemp, 5, 1, tempString);
  Serial.print("Humidity: "); Serial.println(DHThumidity); //print the value to serial
  Serial.print("Temperature: "); Serial.println(DHTtemp); //print the value to serial
  
  if (DHTtemp <= 20) {
    digitalWrite(heaterLED, HIGH);
    Serial.println("Heater LED on!");
  }
  else {
    digitalWrite(heaterLED, LOW);
    Serial.println("Heater LED off!");
  }

  //CO2 Read
  //Read voltage
  int CO2Volt = analogRead(CO2In);
  Serial.print("Raw:"); Serial.println(CO2Volt);

  // The analog signal is converted to a voltage
  float CO2Calc = CO2Volt * (5000 / 1024.0);

  if (CO2Calc == 0)
  {
    Serial.println("CO2 Fault");
  }
  else if (CO2Calc < 400)
  {
    Serial.print(CO2Volt); Serial.println(" CO2 preheating");
  }
  else
  {
    int voltage_diference = CO2Calc - 400;
    CO2 = voltage_diference * 50.0 / 16.0;
    // Print Voltage
    Serial.print("voltage:");
    Serial.print(CO2Calc);
    Serial.println("mv");
    //Print CO2 concentration
    Serial.print(CO2);
    Serial.println("ppm");
    //itoa(CO2, tempString, 12);
    dtostrf(CO2, 7, 0, CO2String);
    //Serial.print("String:"); Serial.println(tempString);
  }
  // picture loop
  u8g.firstPage();
  do {
    draw();
  }
  while ( u8g.nextPage() );

  delay(10000);
}

void draw(void) {

  lcd.setCursor(0, 0);
  lcd.print("CO2:");
  lcd.print(CO2String);
  lcd.print("ppm");

  lcd.setCursor(0, 1);
  lcd.print("Temp: ");
  lcd.print(tempString);
  lcd.print("C");

  lcd.setCursor(0, 2);
  lcd.print("Humidity: ");
  lcd.print(humidityString);
  lcd.print("%");

  lcd.setCursor(0, 3);
  lcd.print("Light: ");
  lcd.print(light);



  // graphic commands to redraw the complete screen should be placed here
  u8g.setFont(u8g_font_unifont);
  //u8g.setFont(u8g_font_osb21);
  u8g.drawStr( 0, 20, "CO2 PPM: ");
  u8g.drawStr( 70, 20, CO2String);

  u8g.drawStr( 0, 34, "Temp: ");
  u8g.drawStr( 70, 34, tempString);

  u8g.drawStr( 0, 48, "Humidity:");
  u8g.drawStr( 70, 48, humidityString);
  u8g.drawStr( 82, 48, "%");

  u8g.drawStr( 0, 62, "Light: ");
  u8g.drawStr( 70, 62, lightString);

}




