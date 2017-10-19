/*
  Done by TATCO Inc.
  Contacts:
  info@tatco.cc

  Release Notes:
  - V1 Created 10 Oct 2017
  
  
  Connection:
  - No connections are required, Just uplode the sketch to you Arduino 101.

  Requirments:
  - Mobile App "Arduino 101 Kit" on both OS:
      iOS: 
      Android: 
  - 

  
*/


#include <CurieBLE.h>
#include <Servo.h>


BLEPeripheral blePeripheral;
BLEService serviceCharacteristic("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");
BLECharacteristic txRxCharacteristic("6E400002-B5A3-F393-E0A9-E50E24DCCA9E", BLEWrite | BLENotify | BLERead , 20);

char Buffer[32] ;

#define lcd_size 3 //this will define number of LCD on the phone app
int refresh_time = 3; //the data will be updated on the app every 3 seconds.

char mode_action[54];
int mode_val[54];
String mode_feedback ;
String lcd[lcd_size];
unsigned long last = millis();
Servo myServo[54];

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
  
  kitSetup();

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
    analogCommand(second, third);
  }

  if (command == "mode") {//to chang the mode of the pin.
    modeCommand(second, third);
  }

  if (command == "servo") {// to control servo(0°-180°).
    servo(second, third);
  }

  if (command == "allonoff") {//to turn all pins on or off.
    allonoff(second, third);
  }
  if (command == "refresh") {// to change the refresh time.
    refresh(second);
  }

  if (command == "allstatus") {// send JSON object arduino includes all data.
    allstatus();
  }
}

void terminalCommand(String second) {//Here you recieve data form app terminal
  String data = second;
  lcd[2] = data;//show data on LCD #2
  Serial.println(data);
}

void digitalCommand(String second, String third) {
  int pin, value;
  pin = second.toInt();
  value = third.toInt();

  digitalWrite(pin, value);
  mode_val[pin] = value;

}

void analogCommand(String second, String third) {
  int pin, value;
  pin = second.toInt();
  value = third.toInt();

  analogWrite(pin, value);
  mode_val[pin] = value;


}

void servo(String second, String third) {
  int pin, value;
  pin = second.toInt();
  value = third.toInt();
  myServo[pin].write(value);
  mode_val[pin] = value;

}

void modeCommand(String second, String third ) {
  int pin = second.toInt();
  String mode = third;
  mode_feedback = "";
  //  Serial.print(pin);
  Serial.println(mode.length());
  if (mode == "input") {
    pinMode(pin, INPUT);
    mode_action[pin] = 'i';
    mode_feedback += "D";
    mode_feedback += pin;
    mode_feedback += " set as INPUT!";

  }

  if (mode == "output") {
    pinMode(pin, OUTPUT);
    mode_action[pin] = 'o';
    mode_feedback += "D";
    mode_feedback += pin;
    mode_feedback += " set as OUTPUT!";
  }

  if (mode == "pwm") {
    pinMode(pin, OUTPUT);
    mode_action[pin] = 'p';
    mode_feedback += "D";
    mode_feedback += pin;
    mode_feedback += " set as PWM!";
  }

  if (mode == "servo") {
    myServo[pin].attach(pin);
    mode_action[pin] = 's';
    mode_feedback += "D";
    mode_feedback += pin;
    mode_feedback += " set as SERVO!";
  }
  allstatus();

}

void allonoff(String second, String third) {
  //  int pin, value;
  //  pin = second.toInt();
  int value = second.toInt();
  for (byte i = 0; i < sizeof(mode_action); i++) {
    if (mode_action[i] == 'o') {
      digitalWrite(i, value);
      mode_val[i] = value;
    }
  }
}

void refresh(String second) {
  int value = second.toInt();
  refresh_time = value;
  Serial.println(value);

}



void updateInputs() {
  for ( unsigned int i = 0; i < sizeof(mode_action); i++) {
    if (mode_action[i] == 'i') {
      mode_val[i] = digitalRead(i);
    }
  }
}

void updateApp() {

  if (refresh_time != 0) {
    unsigned int refreshVal = refresh_time * 1000;
    if (millis() - last > refreshVal) {
      allstatus();
      last = millis();
    }
  }
}

void allstatus() {

  String data_status;
  data_status += "{";
  data_status += "\"m\":[";//m for pin mode
  for (byte i = 0; i <= 13; i++) {
    data_status += "\"";
    data_status += mode_action[i];
    data_status += "\"";
    if (i != 13)data_status += ",";
  }
  data_status += "],";

  data_status += "\"v\":[";//v for mode value
  for (byte i = 0; i <= 13; i++) {
    data_status += mode_val[i];
    if (i != 13)data_status += ",";
  }
  data_status += "],";

  data_status += "\"a\":[";//a for analog
  for (byte i = 0; i <= 5; i++) {
    data_status += analogRead(i);
    if (i != 5)data_status += ",";
  }
  data_status += "],";

  data_status += "\"l\":[";// for lcd
  for (byte i = 0; i <= lcd_size - 1; i++) {
    data_status += "\"";
    data_status += lcd[i];
    data_status += "\"";
    if (i != lcd_size - 1)data_status += ",";
  }
  data_status += "],";

  data_status += "\"f\":\"";// for feedback.
  data_status += mode_feedback;
  data_status += "\",";
  data_status += "\"t\":\"";//t for time.
  data_status +=  refresh_time;
  data_status += "\"";
  data_status += "}";
  char dataBuffer[20];

  int data_statusLength = data_status.length();
  for (int i = 0; i <= data_statusLength; i = i + 20) {
    for (int x = 0; x < 20; x++) {
      dataBuffer[x] = data_status[i + x];
    }
    txRxCharacteristic.setValue((unsigned char *)dataBuffer, 20) ;
  }
  txRxCharacteristic.setValue((unsigned char *)"\n", 2) ;
  mode_feedback = "";
}

void kitSetup() {
  for (byte i = 0; i <= 13; i++) {
    if (i == 0 || i == 1 ) {
      mode_action[i] = 'x';
      mode_val[i] = 0;
    }
    else {
      mode_action[i] = 'o';
      mode_val[i] = 0;
      pinMode(i, OUTPUT);
    }
  }
}
