/*
 * Project:     Midterm Smart Room Controller
 * Description: Controlling smart things in the room
 * Name:        Miguel Lopez
 * Date:        07/28/20
 */

#include <SPI.h>
#include <Ethernet.h>
#include <Encoder.h>
#include <OneButton.h>
#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include <colors.h>
#include <hue.h>
#include <mac.h>
#include <wemo.h>

#include <Adafruit_BME280.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

#define OLED_RESET    4
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

Adafruit_BME280 bme;
float tempF;
float pressinHg;
float humidRH;
int Tempvalue;
bool status;


Encoder myEnc(17,16);
OneButton encButton(20, false);
int buttonState = LOW;
int redPin = 21;
int greenPin = 22;
int ticks;
int dial;
int pixdial;
int lockPos;

OneButton greenButton(6, false);
int buttonState2 = LOW;

const int NeoPin = 14;
const int PixelNum = 16;
Adafruit_NeoPixel pixels(PixelNum, NeoPin, NEO_GRB + NEO_KHZ800);


IPAddress ip(192,168,1,18);  // Teensy IP
boolean activated;  // true for bulb on, falso for off
int bulb = 1;           // select which bulb (1 to 5)
int color;         // select a color (4-byte format)
int bri;            // select brightness (0-255)
int Huebri = 265;

int buttonPin = 23;
bool state = true;
bool buttonRead;
int wemo = 3;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  pinMode(buttonPin, INPUT);
  
  pixels.begin();
  pixels.clear();

  pinMode(10, OUTPUT);
  digitalWrite(10, HIGH);
  Ethernet.begin(mac,ip);
  //delay(2000);              // Wait for Serial Monitor
  Serial.print("LinkStatus: ");
  Serial.println(Ethernet.linkStatus());
  Serial.println("Ready.");

  greenButton.attachClick(click2);
  greenButton.setClickTicks(250);
  greenButton.setPressTicks(2000);
  pinMode(6, INPUT);
  
  encButton.attachClick(click1);
  encButton.setClickTicks(250);
  encButton.setPressTicks(2000);
  pinMode(20, INPUT_PULLUP);
  pinMode(21, OUTPUT);
  pinMode(22, OUTPUT);

  status = bme.begin(0x76);
  if(status == false) {
    Serial.printf("Check your wires dummy\n");
  }
  else
  {
    Serial.printf("BME is GOOD\n");
  }

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

}

void loop() {
  // put your main code here, to run repeatedly:

  BME();
  HUE();
  WEMO();
}


void HUE() {
  encButton.tick();
    if(buttonState == LOW) {
      digitalWrite(redPin, LOW);
      digitalWrite(greenPin, HIGH);
      myEnc.write(lockPos);
      activated = false; 
      setHue(bulb,activated, 0,0);
      pixels.show();
      pixels.clear();
    }
    else
    {
      digitalWrite(redPin, HIGH);
      digitalWrite(greenPin, LOW);
      lockPos = myEnc.read();
      activated = true;
      //bulb = random(1,6);
      setHue(bulb,activated, HueRed, dial);
      //color++;
      
      bri = myEnc.read();
      dial = map(bri, 0, 96, 0, 265);
      pixdial = map(dial, 0, 265, 0, 15);
      pixels.setBrightness(10);
      pixels.fill(yellow, 0, pixdial+1);
      pixels.show();
      pixels.clear();
      
      Serial.println(dial);
      if(bri>96){
        myEnc.write(96);
      }
        if(bri<0){
        myEnc.write(0);
        }
    }
   if (color>6) {
    color = 0;
  }
}


void BME() {
  greenButton.tick();
  if(buttonState2 == LOW) {
    pixels.clear();
    pixels.show();
    }
   else
    {
    tempF = (bme.readTemperature() * 8/5) + 32;
    pressinHg = (bme.readPressure() * 0.00029530);
    humidRH = bme.readHumidity();
    Values(tempF, pressinHg, humidRH);
    //delay(5000);
    
    Tempvalue = map(tempF, 65, 80,1,15);
    pixels.setBrightness(10);
    pixels.fill(blue, 0, Tempvalue);
    if(Tempvalue > 3){
      pixels.fill(orange, 3, Tempvalue - 3);
      }
    if(Tempvalue > 8) {
      pixels.fill (red, 8, Tempvalue - 8);
      }
   
  pixels.show();
  }
}

void Values(float VtempF, float VpressinHg, float VhumidRH) {

    display.clearDisplay();

    display.setRotation(2);
    display.setTextSize(1);
    display.setCursor(0,0);
    display.setTextColor(SSD1306_WHITE);

    //Serial.printf("Temp. = %0.2f *F\n", VtempF);
    display.printf("Temp. = %0.2f *F\n", VtempF);

    //Serial.printf("Press. = %0.2f inHg\n", VpressinHg);   
    display.printf("Press. = %0.2f inHg\n", VpressinHg);   
    
    //Serial.printf("Humi. = %0.2f \n", VhumidRH);
    display.printf("Humi. = %0.2f \n", VhumidRH);

    Serial.printf("\n");
    display.display();
}

void WEMO() {
  buttonRead = digitalRead(buttonPin);
   if (buttonRead != state){
     if (buttonRead==true){
       switchON(wemo);
       //delay(1000);
     }else{
       switchOFF(wemo);
       //wemo++;
       //delay(1000);
     }
     state = buttonRead;
   }
}

void click2() {
  buttonState2 = (!buttonState2);
}

void click1() {
  buttonState = (!buttonState);
}
