/*
  Title  : Arduino 101 Kit
  version: V4.1
  Contact: info@tatco.cc
  Done By: TATCO Inc.
  github : https://github.com/rabee2050/arduino-101
  Youtube: http://tatco.cc

  Apps:
  iOS    : https://itunes.apple.com/us/app/arduino-101-kit/id1298510087?ls=1&mt=8
  Android: https://play.google.com/store/apps/details?id=com.tatco.curie&hl=en

  Release Notes:
  - V1 Created 10 Oct 2017
  - V2 skipped
  - V3 skipped
  - V4 Updated 18 Dec 2018
  - V4.1 Updated 06 Apr 2019 / Minor Changes


  Connection:
  - No connections are required, Just upload the sketch to you Arduino 101.

*/


#include <CurieBLE.h>
#include <Servo.h>


BLEPeripheral blePeripheral;
BLEService serviceCharacteristic("6e400001-b5a3-f393-e0a9-e50e24dcca9e");
BLECharacteristic txRxCharacteristic("6e400002-b5a3-f393-e0a9-e50e24dcca9e", BLEWrite | BLENotify | BLERead , 20);
char Buffer[32] ;

#define lcdSize 3 //this will define number of LCD on the phone app
int refreshTime = 3; //the data will be updated on the app every 3 seconds.

char pinsMode[54];
int pinsValue[54];
String feedBack ;
String lcd[lcdSize];
String boardType;
String protectionPassword = "";
String appBuildVersion = "4.1";

unsigned long last = millis();
Servo servoArray[54];

void setup()
{
  Serial.begin(9600);
  // Set advertised local name and service UUID:
  blePeripheral.setLocalName("Arduino101");
  blePeripheral.setDeviceName("Arduino101");
  blePeripheral.setAppearance(true);
  blePeripheral.setAdvertisedServiceUuid(serviceCharacteristic.uuid());
  blePeripheral.addAttribute(serviceCharacteristic);
  blePeripheral.addAttribute(txRxCharacteristic);
  blePeripheral.begin();
  Serial.println("Arduino 101 is running and waiting for connection.....");
  Serial.println("");
  boardInit();
}

void loop()
{
  BLE.poll();
  // listen for BLE peripherals to connect:
  BLECentral central = blePeripheral.central();
  // if a central is connected to peripheral:
  if (central)
  {
    // print the central's MAC address:
    Serial.print("Connected to mobile app with the address: ");
    Serial.println(central.address());

    while (central.connected())
    {
      if (txRxCharacteristic.written())  //check if the phone send data.
      {
        String dataIncoming = (char*)txRxCharacteristic.value();//buffer the incomming data
        /*split the incoming data into usfull command*/
        int commaIndex = dataIncoming.indexOf('/');
        int secondCommaIndex = dataIncoming.indexOf('/', commaIndex + 1);
        int thirdCommaIndex = dataIncoming.indexOf('/', secondCommaIndex + 1);

        String first = dataIncoming.substring(0, commaIndex);//Commands like: mode, digital, analog, servo,etc...
        String second = dataIncoming.substring(commaIndex + 1, secondCommaIndex);//Pin number.
        String third = dataIncoming.substring(secondCommaIndex + 1, thirdCommaIndex );// value of the pin.
        String forth = dataIncoming.substring(thirdCommaIndex + 1 );//not used,it is for future provision.
        /*------------------------------------------*/
        process(first, second, third, forth);//
      }
      updateInputs();//if it is input then update pin value.
      updateApp();//Send data to mobile app every specidic time

      lcd[0] = "Test 1 LCD";// you can send any data to your mobile app.
      lcd[1] = "Test 2 LCD2";// you can send any data to your mobile app.
      lcd[2] = 85;// you can send any data to your mobile app.

    }

    // when the central disconnects, print it out:
    Serial.print("Disconnected from mobile app: ");
    Serial.println(central.address());
    Serial.println();

  }
}

void process(String command, String second, String third, String forth) {

  if (command == "terminal") {//to recieve data from mobile app from terminal text box
    terminalCommand(second);
  }

  if (command == "digital") {//to turn pins on or off
    digitalCommand(second, third);
  }

  if (command == "analog") {//to write analog value(PWM).
    pwmCommand(second, third);
  }

  if (command == "mode") {//to chang the mode of the pin.
    modeCommand(second, third);
  }

  if (command == "servo") {// to control servo(0°-180°).
    servoCommand(second, third);
  }

  if (command == "allonoff") {//to turn all pins on or off.
    allonoff(second, third);
  }
  if (command == "refresh") {// to change the refresh time.
    refresh(second);
  }

  if (command == "allstatus") {// send JSON object arduino includes all data.
    feedBack = "refresh/";
    allstatus();
  }
}

void terminalCommand(String second) {//Here you recieve data form app terminal
  String data = second;
  Serial.println(data);

  String replyToApp = "Ok from Arduino"; //It can be change to any thing

  feedBack = "terminal/" + replyToApp; //dont change this line.
  allstatus();
}

void digitalCommand(String second, String third) {
  int pin, value;
  pin = second.toInt();
  value = third.toInt();

  digitalWrite(pin, value);
  pinsValue[pin] = value;

}

void pwmCommand(String second, String third) {
  int pin, value;
  pin = second.toInt();
  value = third.toInt();

  analogWrite(pin, value);
  pinsValue[pin] = value;


}

void servoCommand(String second, String third) {
  int pin, value;
  pin = second.toInt();
  value = third.toInt();
  servoArray[pin].write(value);
  pinsValue[pin] = value;

}

void modeCommand(String second, String third ) {
  int pin = second.toInt();
  String mode = third;

  if (mode == "output") {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, 0);
    pinsMode[pin] = 'o';
    pinsValue[pin] = 0;
  }
  if (mode == "push") {
    pinsMode[pin] = 'm';
    pinsValue[pin] = 0;
    pinMode(pin, OUTPUT);
    digitalWrite(pin, 0);
  }
  if (mode == "schedule") {
    pinsMode[pin] = 'c';
    pinsValue[pin] = 0;
    pinMode(pin, OUTPUT);
    digitalWrite(pin, 0);
  }

  if (mode == "input") {
    pinsMode[pin] = 'i';
    pinsValue[pin] = 0;
    pinMode(pin, INPUT);
  }

  if (mode == "pwm") {
    pinsMode[pin] = 'p';
    pinsValue[pin] = 0;
    pinMode(pin, OUTPUT);
    analogWrite(pin, 0);
  }

  if (mode == "servo") {
    pinsMode[pin] = 's';
    pinsValue[pin] = 0;
    servoArray[pin].attach(pin);
    servoArray[pin].write(0);
  }
  feedBack = "mode/" + mode + "/" + pin + "/" + pinsValue[pin];
  allstatus();
}

void allonoff(String second, String third) {
  //  int pin, value;
  //  pin = second.toInt();
  int value = third.toInt();
  for (byte i = 0; i < sizeof(pinsMode); i++) {
    if (pinsMode[i] == 'o') {
      digitalWrite(i, value);
      pinsValue[i] = value;
    }
  }
}

void refresh(String second) {
  int value = second.toInt();
  refreshTime = value;
  Serial.println(value);
  allstatus();
}



void updateInputs() {
  for ( unsigned int i = 0; i < sizeof(pinsMode); i++) {
    if (pinsMode[i] == 'i') {
      pinsValue[i] = digitalRead(i);
    }
  }
}

void updateApp() {

  if (refreshTime != 0) {
    unsigned int refreshVal = refreshTime * 1000;
    if (millis() - last > refreshVal) {
      allstatus();
      last = millis();
    }
  }
}

void allstatus() {

  String dataResponse;
  dataResponse += "{";
  dataResponse += "\"m\":[";//m for pin mode
  for (byte i = 0; i <= 13; i++) {
    dataResponse += "\"";
    dataResponse += pinsMode[i];
    dataResponse += "\"";
    if (i != 13)dataResponse += ",";
  }
  dataResponse += "],";

  dataResponse += "\"v\":[";//v for mode value
  for (byte i = 0; i <= 13; i++) {
    dataResponse += pinsValue[i];
    if (i != 13)dataResponse += ",";
  }
  dataResponse += "],";

  dataResponse += "\"a\":[";//a for analog
  for (byte i = 0; i <= 5; i++) {
    dataResponse += analogRead(i);
    if (i != 5)dataResponse += ",";
  }
  dataResponse += "],";

  dataResponse += "\"l\":[";// for lcd
  for (byte i = 0; i <= lcdSize - 1; i++) {
    dataResponse += "\"";
    dataResponse += lcd[i];
    dataResponse += "\"";
    if (i != lcdSize - 1)dataResponse += ",";
  }
  dataResponse += "],";

  dataResponse += "\"t\":\""; //t for Board Type .
  dataResponse += "101";
  dataResponse += "\",";
  dataResponse += "\"f\":\""; //t for Board Type .
  dataResponse += feedBack;
  dataResponse += "\",";
  dataResponse += "\"r\":\""; //t for Board Type .
  dataResponse += refreshTime;
  dataResponse += "\",";
  dataResponse += "\"b\":\""; //b for app build version .
  dataResponse += appBuildVersion;
  dataResponse += "\",";
  dataResponse += "\"p\":\""; // p for Password.
  dataResponse += protectionPassword;
  dataResponse += "\"";
  dataResponse += "}";
  
  char dataBuffer[20];
  int dataResponseLength = dataResponse.length();
  
  for (int i = 0; i <= dataResponseLength; i = i + 20) {
    for (int x = 0; x < 20; x++) {
      dataBuffer[x] = dataResponse[i + x];
    }
    txRxCharacteristic.setValue((unsigned char *)dataBuffer, 20) ;
  }
  txRxCharacteristic.setValue((unsigned char *)"\n", 2) ;
  feedBack = "";
}

void boardInit() {
  for (byte i = 0; i <= 13; i++) {
    if (i == 0 || i == 1 ) {
      pinsMode[i] = 'x';
      pinsValue[i] = 0;
    }
    else {
      pinsMode[i] = 'o';
      pinsValue[i] = 0;
      pinMode(i, OUTPUT);
    }
  }
}

