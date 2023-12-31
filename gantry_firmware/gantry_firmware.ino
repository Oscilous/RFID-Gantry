#include <SoftwareSerial.h>
#include "SparkFun_UHF_RFID_Reader.h" //Library for controlling the M6E Nano module

#define POSITIVE 1
#define NEGATIVE 0
#define MAX_X 250
#define MAX_Y 340
#define SCAN_AMOUNT 1
#define RFID_STRENGHT 2000
#define SEARCH_GRID_X 5
#define SEARCH_GRID_Y 5

//Denominators
//For 250: 1, 2, 5, 10, 25, 50, 125, 250
//For 340: 1, 2, 4, 5, 10, 17, 20, 34, 68, 85, 170, 340 ​

const int TIME = 100;

const int StepX = 2;
const int DirX = 5;
const int StepY = 3;
const int DirY = 6;

int endSwitchX = 9;
int endSwitchY = 10;

float currentX = 0;
float currentY = 0;

int incomingByte = 0;
void move_stepper(int axis, char direction, long int steps, int speed);
void auto_home(void);
void send_current_pos(int scanning);
void search(int width);
boolean setupNano(long baudRate);
void setup_RFID(void);
int read_tags(void);
int process_signal_strenght(int value);
void deep_search(void);

//RFID
SoftwareSerial softSerial(12, 13); //RX, TX
RFID nano; //Create instance

void setup() {
  Serial.begin(115200);
  while (!Serial); //Wait for the serial port to come online
  Serial.println("Arduino started");
  pinMode(StepX,OUTPUT);
  pinMode(DirX,OUTPUT);
  pinMode(StepY,OUTPUT);
  pinMode(DirY,OUTPUT);

  pinMode(endSwitchX, INPUT_PULLUP);
  pinMode(endSwitchY, INPUT_PULLUP);

  setup_RFID();
  Serial.print("RFID temp: ");
  Serial.println(nano.getTemp());
  nano.stopReading();
  Serial.print("begin");
}

void loop() {
  incomingByte = Serial.read();
  if (incomingByte == 49){ // 49 is ascii for 1
    auto_home();
  }
  else if (incomingByte == 50){ //50 is ascii for 2
    nano.startReading();
    search(2);
    nano.stopReading();
  }
  else if (incomingByte == 51){ //51 is ascii for 3
    Serial.print("Reading");
    int signal_strenght = read_tags();
    Serial.print("Stopped");
  }
}

void auto_home(void){
  digitalWrite(DirX, LOW); // set direction, HIGH for clockwise (positive x, neg for y), LOW for anticlockwise 
  while (digitalRead(endSwitchX) == 0){
    digitalWrite(StepX,HIGH);
    delayMicroseconds(TIME);
    digitalWrite(StepX,LOW); 
    delayMicroseconds(TIME);
  }
  digitalWrite(DirY, HIGH); // set direction, HIGH for clockwise, LOW for anticlockwise 
  while (digitalRead(endSwitchY) == 0){
    digitalWrite(StepY,HIGH);
    delayMicroseconds(TIME);
    digitalWrite(StepY,LOW); 
    delayMicroseconds(TIME);
  }
  currentX = 0;
  currentY = 0;
  send_current_pos();
}

void move_stepper(int axis, char direction, long int steps, int speed){
  if (direction == 1 && axis == StepX){
    if (currentX + steps > 10000){
      Serial.println("Out of bounds");
      return;
    }
    digitalWrite(DirX, HIGH); // set direction, HIGH for clockwise (positive x, neg for y), LOW for anticlockwise 
    currentX = currentX + steps;
  }
  else if (direction == 0 && axis == StepX){
    if (currentX - steps < 0){
      Serial.println("Out of bounds");
      return;
    }
    digitalWrite(DirX, LOW); // set direction, HIGH for clockwise (positive x, neg for y), LOW for anticlockwise 
    currentX = currentX - steps;
  }
  else if (direction == 1 && axis == StepY){
    if (currentY + steps > 68000){
      Serial.println("Out of bounds");
      return;
    }
    digitalWrite(DirY, LOW); // set direction, HIGH for clockwise (positive x, neg for y), LOW for anticlockwise 
    currentY = currentY + steps;
  }
  else if (direction == 0 && axis == StepY){
    if (currentY - steps < 0){
      Serial.println("Out of bounds");
      return;
    }
    digitalWrite(DirY, HIGH); // set direction, HIGH for clockwise (positive x, neg for y), LOW for anticlockwise 
    currentY = currentY - steps;
  }
  send_current_pos();
  if (axis == StepX){
    steps = steps * 40;
  }
  if (axis == StepY){
    steps = steps * 200;
  }
  for (long int i = 0; i < steps; i++){
    digitalWrite(axis,HIGH);
    delayMicroseconds(speed);
    digitalWrite(axis,LOW); 
    delayMicroseconds(speed);
  }
}

void send_current_pos(){
  for (int i = 0; i<1; i++){
    Serial.print("x=");
    Serial.print(int(currentX));
    Serial.print(";");
    Serial.print("y=");
    Serial.print(int(currentY));
    Serial.print(";");
    Serial.print("r=255;g=255;b=255");
    Serial.print("$");
  }
}

void search(int width){
  int x_gap = MAX_X / SEARCH_GRID_X;
  int y_gap = MAX_Y / SEARCH_GRID_Y;
  int tag_found = 0;
  for (int j = 0; j < x_gap - 1;j++){
      move_stepper(StepX, POSITIVE, 1, 200);
  }
  for(int k=0; k < ((SEARCH_GRID_Y - 1) / 2);k++){
    for (int j = 0; j < y_gap - 1;j++){
        move_stepper(StepY, POSITIVE, 1, 200);
    }
    //Search
    if (read_tags() != 0){
      tag_found = 1;
      break;
    }
    //Start grid
    for (int i = 0; i < SEARCH_GRID_X - 2;i++){
      for (int j = 0; j < x_gap - 1;j++){
          move_stepper(StepX, POSITIVE, 1, 200);
      } 
      if (read_tags() != 0){
        tag_found = 1;
        break;
      }   
    }
    if (tag_found == 1){
      break;
    }
    //GO one up
    for (int j = 0; j < y_gap - 1;j++){
        move_stepper(StepY, POSITIVE, 1, 200);
    }
    if (read_tags() != 0){
      tag_found = 1;
      break;
    }
    for (int i = 0; i < SEARCH_GRID_X - 2;i++){
      for (int j = 0; j < x_gap - 1;j++){
          move_stepper(StepX, NEGATIVE, 1, 200);
      } 
      if (read_tags() != 0){
        tag_found = 1;
        break;
      }
    }
    if (tag_found == 1){
      break;
    }
  }
  read_tags();
  deep_search();
}

void deep_search(void) {
  
  move_stepper(StepY, POSITIVE, 1, 200);
  if (read_tags() == 0){
    move_stepper(StepX, POSITIVE, 1, 200);
    if (read_tags() == 0){
      move_stepper(StepY, NEGATIVE, 1, 200);
      if (read_tags() == 0){
        move_stepper(StepX, NEGATIVE, 1, 200);
        if (read_tags() == 0){
          move_stepper(StepY, NEGATIVE, 1, 200);
        }
      }
    }
  }
  read_tags();
}

//Gracefully handles a reader that is already configured and already reading continuously
//Because Stream does not have a .begin() we have to do this outside the library
boolean setupNano(long baudRate)
{
  nano.begin(softSerial); //Tell the library to communicate over software serial port

  //Test to see if we are already connected to a module
  //This would be the case if the Arduino has been reprogrammed and the module has stayed powered
  softSerial.begin(baudRate); //For this test, assume module is already at our desired baud rate
  while (softSerial.isListening() == false); //Wait for port to open

  //About 200ms from power on the module will send its firmware version at 115200. We need to ignore this.
  while (softSerial.available()) softSerial.read();

  nano.getVersion();

  if (nano.msg[0] == ERROR_WRONG_OPCODE_RESPONSE)
  {
    //This happens if the baud rate is correct but the module is doing a ccontinuous read
    nano.stopReading();

    Serial.println(F("Module continuously reading. Asking it to stop..."));

    delay(1500);
  }
  else
  {
    //The module did not respond so assume it's just been powered on and communicating at 115200bps
    softSerial.begin(115200); //Start software serial at 115200

    nano.setBaud(baudRate); //Tell the module to go to the chosen baud rate. Ignore the response msg

    softSerial.begin(baudRate); //Start the software serial port, this time at user's chosen baud rate

    delay(250);
  }

  //Test the connection
  nano.getVersion();
  if (nano.msg[0] != ALL_GOOD) return (false); //Something is not right

  //The M6E has these settings no matter what
  nano.setTagProtocol(); //Set protocol to GEN2

  nano.setAntennaPort(); //Set TX/RX antenna ports to 1

  return (true); //We are ready to rock
}

void setup_RFID(void){
  //Setup RFID
  if (setupNano(38400) == false) //Configure nano to run at 38400bps
  {
    Serial.println(F("Module failed to respond. Please check wiring."));
    while (1); //Freeze!
  }

  nano.setRegion(REGION_EUROPE); //Set to North America

  nano.setReadPower(RFID_STRENGHT); // 500 = 5.00 dBm. Higher values may caues USB port to brown out
  //Max Read TX Power is 2700 = 27.00 dBm and may cause temperature-limit throttling

}

int read_tags(void){
    int scans = 0;
    int rssi_buffer = 100; 
    while(scans < SCAN_AMOUNT){
      if (nano.check() == true) //Check to see if any new data has come in from module
      {
        scans++;
        byte responseType = nano.parseResponse(); //Break response into tag ID, RSSI, frequency, and timestamp
      
        if (responseType == RESPONSE_IS_KEEPALIVE)
        {
          //Serial.println(F("Scanning"));
        }
        else if (responseType == RESPONSE_IS_TAGFOUND)
        {
          //If we have a full record we can pull out the fun bits
          int rssi = nano.getTagRSSI(); //Get the RSSI for this tag read
          if (rssi_buffer > rssi){
            rssi_buffer = rssi;
          }
        }
        else if (responseType == ERROR_CORRUPT_RESPONSE)
        {
          //Serial.println("Bad CRC");
        }
        else
        {
          //Unknown response
          //Serial.print("Unknown error");
        }
      }
    }
    rssi_buffer = process_signal_strenght(rssi_buffer);
    if (rssi_buffer != 0){
      Serial.print("x=");
      Serial.print(int(currentX));
      Serial.print(";");
      Serial.print("y=");
      Serial.print(int(currentY));
      Serial.print(";");
      Serial.print("r=");
      Serial.print(rssi_buffer);
      Serial.print(";g=0;b=0");
      Serial.print("$");
    }
    //Serial.print(rssi_buffer);
    return rssi_buffer;
}

int process_signal_strenght(int value){
  if (value == 100){
      return 0;
    }
    else{
      return int((value / 5 * 51) + 663);
    }
}
/*
int8_t t = nano.getTemp();

if (t == -1 ){
    Serial.println(F("Error getting Nano internal temperature"));
}
else {
  Serial.print(F("Nano internal temperature (*C): "));
  Serial.println(t);
}
*/