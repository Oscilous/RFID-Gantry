#define POSITIVE 1
#define NEGATIVE 0
#define MAX_X 250
#define MAX_Y 340

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
void send_current_pos(void);

void setup() {
  Serial.begin(9600);
  pinMode(StepX,OUTPUT);
  pinMode(DirX,OUTPUT);
  pinMode(StepY,OUTPUT);
  pinMode(DirY,OUTPUT);

  pinMode(endSwitchX, INPUT_PULLUP);
  pinMode(endSwitchY, INPUT_PULLUP);

}

void loop() {
  incomingByte = Serial.read();
  if (incomingByte == 49){ // 49 is ascii for 1
    auto_home();
  }
  else if (incomingByte == 50){ //50 is ascii for 2
    scan(2);
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

void send_current_pos(void){
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

void scan(int width){
  width = 170 / width;
  for (int i = 0; i < (MAX_Y / width / 2); i++){
    for (int j = 0; j < MAX_X - 1;j++){
      move_stepper(StepX, POSITIVE, 1, 200);
    }
    for (int j = 0; j < width; j++){
      move_stepper(StepY, POSITIVE, 1, 200);
    }
    for (int j = 0; j < MAX_X - 1;j++){
      move_stepper(StepX, NEGATIVE, 1, 200);
    }
    for (int j = 0; j < width; j++){
      move_stepper(StepY, POSITIVE, 1, 200);
    }
  }
}


