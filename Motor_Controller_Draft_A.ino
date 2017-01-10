// Motor Controller with I2C Interface 
//
// Required Data:
// 
// Motor Number 
// Motor Direction
// Motor Speed 
//
// Motor A connected between A01 and A02
// Motor B connected between B01 and B02
//
// Includes 

#include <Wire.h>

// Wire Variables

#define SLAVE_ADDRESS 0x04

int messageLength = 0;
int commandReceived = 0; 

int SpeedLeft = 0;
int SpeedRight = 0;

int DirectionLeft = 0;
int DirectionRight = 0; 

// Motor Variable 

int STBY = 4; //standby

//Motor A
int PWMA = 3; //Speed control 
int AIN1 = 2; //Direction
int AIN2 = 7; //Direction

//Motor B
int PWMB = 6; //Speed control
int BIN1 = 8; //Direction
int BIN2 = 9; //Direction

void setup()
{
  Wire.begin(SLAVE_ADDRESS); 
  Wire.onReceive(receiveData);
  Wire.onRequest(transmitData); 
  
  pinMode(STBY, OUTPUT);

  pinMode(PWMA, OUTPUT);
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);

  pinMode(PWMB, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
}

void loop()
{
  move(1, SpeedLeft, DirectionLeft); //motor 1, full speed, left
  move(2, SpeedRight, DirectionRight); //motor 2, full speed, left
  
  delay(10);
}


void move(int motor, int speed, int direction)
{
// Motor 1 = Left 
// Motor 2 = Right
// Direction 0 = Forward
// Direction 1 = Back 
// 
// Move specific motor at speed and direction
// motor: 0 for B 1 for A
// speed: 0 is off, and 255 is full speed
// direction: 0 clockwise, 1 counter-clockwise

  digitalWrite(STBY, HIGH); //disable standby

  boolean inPin1 = LOW;
  boolean inPin2 = HIGH;

  if(direction == 1)
  {
    inPin1 = HIGH;
    inPin2 = LOW;
  }

  if(motor == 1){
    digitalWrite(AIN1, inPin1);
    digitalWrite(AIN2, inPin2);
    analogWrite(PWMA, speed);
  }else{
    digitalWrite(BIN1, inPin1);
    digitalWrite(BIN2, inPin2);
    analogWrite(PWMB, speed);
  }
}

void stop()
{
//enable standby  
  digitalWrite(STBY, LOW); 
}

// I2C Callbacks

void receiveData(int byteCount)
{
  messageLength = Wire.read();
  commandReceived = Wire.read(); 
  
  byte dataBytes [messageLength -2];
  
  switch (commandReceived)
  {
    case 0:
    {
      stop();
      break;
    }
    
    case 1: 
    {
      for (int m = 0; m < messageLength -2; m++)
      {
        dataBytes[m] = Wire.read();        
      }
      
      SpeedLeft = dataBytes[0];
      SpeedRight = dataBytes[1];
      DirectionLeft = dataBytes[2];
      DirectionRight = dataBytes[3];   
      
      break;           
    }
  }
}

void transmitData()
{
  
}
