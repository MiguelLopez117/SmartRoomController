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
Adafruit_SSD1306 display2(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

Adafruit_BME280 bme;
float tempF;
float pressinHg;
float humidRH;
int tempValue;
bool status;


Encoder myEnc(17,16);
OneButton encButton(20, false);
int pinEncButton = 20;
int buttonState = LOW;
int redPin = 21;
int greenPin = 22;
int ticks;
int dial;
int pixDial;
int lockPos;

OneButton greenButton(6, false);
int pinGreenButton = 6;
int singleClick = false;

OneButton blueButton(5, false);
int pinBlueButton = 5;
int doubleClick = false;


const int NeoPin = 14;
const int PixelNum = 16;
Adafruit_NeoPixel pixels(PixelNum, NeoPin, NEO_GRB + NEO_KHZ800);


IPAddress ip(192,168,1,21);  // Teensy IP
boolean activated;  // true for bulb on, falso for off
int bulb = 1;         // select which bulb (1 to 5)
int color;         // select a color (4-byte format)
int bri;            // select brightness (0-255)
int Huebri = 265;
int hueArray[] = {HueBlue, HueYellow, HueRed};

int pinBlackButton = 23;
bool state = true;
bool buttonRead;
int wemo = 0;
int startTime;
int endTime;
int sumTime;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  pinMode(pinBlackButton, INPUT);
  
  pixels.begin();
  pixels.clear();

  pinMode(10, OUTPUT);
  digitalWrite(10, HIGH);
  Ethernet.begin(mac,ip);
  Serial.print("LinkStatus: ");
  Serial.println(Ethernet.linkStatus());
  Serial.println("Ready.");

  greenButton.attachClick(greenButtonClick);
  greenButton.setClickTicks(250);
  greenButton.setPressTicks(2000);
  pinMode(pinGreenButton, INPUT);

  blueButton.attachLongPressStart(blueButtonLongPress);
  blueButton.setClickTicks(250);
  blueButton.setPressTicks(2000);
  blueButton.setDebounceTicks(50);
  pinMode(pinBlueButton, INPUT);

  
  encButton.attachClick(encButtonClick);
  encButton.setClickTicks(250);
  encButton.setPressTicks(2000);
  pinMode(pinEncButton, INPUT_PULLUP);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);

  status = bme.begin(0x76);
  
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); //Display Temp Values
  display2.begin(SSD1306_SWITCHCAPVCC, 0x3D); //Display Brightness Values
}

void loop() {
  // put your main code here, to run repeatedly:

  blueButton.tick();
  controlHUE();
  displayBMEValues();
  controlWEMODevices();  
}

void controlHUE() {
  encButton.tick();
    if(buttonState == LOW) {
      digitalWrite(redPin, LOW);
      digitalWrite(greenPin, HIGH);
      myEnc.write(lockPos);
      activated = false; 
      for(int i = 1; i<=6; i++){
        setHue(i,activated, 0,0);
      }
      pixels.show();
      pixels.clear();
    }
    else
    {
      digitalWrite(redPin, HIGH);
      digitalWrite(greenPin, LOW);
      lockPos = myEnc.read();
      activated = true;
      for(int i = 1; i<=6; i++) {
        setHue(i,activated, HueYellow, dial);
      }
      
      bri = myEnc.read();
      dial = map(bri, 0, 96, 0, 265);
      pixDial = map(dial, 0, 265, 0, 15);
      pixels.setBrightness(10);
      pixels.fill(green, 0, pixDial+1);
      pixels.show();
      pixels.clear();
      Values(dial);
      
      if(bri>96){
        myEnc.write(96);
        }
      if(bri<0){
        myEnc.write(0);
        }
    }
}

void Values(float dial) {  //Brightness Values
    display2.clearDisplay();

    display2.setRotation(2);
    display2.setTextSize(2);
    display2.setCursor(0,0);
    display2.setTextColor(SSD1306_WHITE);
    
    display2.printf("Hue Bri. = %0.f \n", dial);

    display2.display();
}

void displayBMEValues() {
  greenButton.tick();
  if(singleClick == LOW) {
    activated = false;
    setHue(bulb, activated, 0,0);
    }
   else
    {
    tempF = (bme.readTemperature() * 8/5) + 32;
    pressinHg = (bme.readPressure() * 0.00029530);
    humidRH = bme.readHumidity();
    Values(tempF, pressinHg, humidRH);

    activated = true;
    tempValue = map(tempF, 65, 75, 0, 2);
    bri = 200;
    for(int i = 1; i<=6; i++) {
      setHue(i,activated,hueArray[tempValue], Huebri);
    }
  }
}

void Values(float VtempF, float VpressinHg, float VhumidRH) {  //Temperature Values
   display.clearDisplay();

    display.setRotation(2);
    display.setTextSize(2);
    display.setCursor(0,0);
    display.setTextColor(SSD1306_WHITE);

    display.printf("Rm Temp. = %0.2f *F\n", VtempF);
    display.display();
}

void controlWEMODevices() {
  buttonRead = digitalRead(pinBlackButton);   
   if (buttonRead != state) {
     if (buttonRead==false) {
       switchOFF(wemo);
      }
      else
      {
       switchON(wemo);
       wemo++;
      }
     state = buttonRead;
    }
    if(wemo > 3) {
      wemo = 0;
     }
}

void encButtonClick() {
  buttonState = (!buttonState);
}

void greenButtonClick() {
  singleClick = (!singleClick);
}

void blueButtonLongPress() {
  singleClick = false;
  buttonState = false;
  display.clearDisplay();
  display2.clearDisplay(); 
  display.display();
  display2.display();

  Serial.print("doubleClick = ");
  Serial.println(doubleClick);
}
