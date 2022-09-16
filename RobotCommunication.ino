#include <Arduino.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define LEADER 1
#define WHEEL_DIAMETER 0.06

char SIG_FWD[] = {'0'};
char SIG_LEFT[] = {'1'};
char SIG_RIGHT[] = {'2'};

#define LEFT_EN 3
#define LEFT_IN 4
#define LEFT_OUT 5

#define RIGHT_EN 6
#define RIGHT_IN 8
#define RIGHT_OUT 7

const byte writeAddress[6] = "00002";
const byte readAddress[6] = "00002";

enum direction
{
	LEFT,
	RIGHT
};

class Motor
{
private:
	int _enPin;
	int _inPin;
	int _outPin;

public:
	Motor() {}
	Motor(int enPin, int inPin, int outPin)
	{
		_enPin = enPin;
		_inPin = inPin;
		_outPin = outPin;
	}

	void driveMotor(int speed)
	{
		bool forward = speed >= 0;

		analogWrite(_enPin, abs(speed));
		digitalWrite(_inPin, forward);
		digitalWrite(_outPin, !forward);
	}
};

class Robot
{
public:
	RF24 radio = RF24(9,10);
	Motor motors[2] = {
		Motor(LEFT_EN, LEFT_IN, LEFT_OUT),
		Motor(RIGHT_EN, RIGHT_IN, RIGHT_OUT)
	};

	void drive(int speed, int time){
		motors[LEFT].driveMotor(speed);
		motors[RIGHT].driveMotor(speed);
		delay(time);
		motors[LEFT].driveMotor(0);
		motors[RIGHT].driveMotor(0);
	}

	void sendData(char *message[]){
		radio.stopListening();
		radio.write(message, sizeof(*message));
		radio.startListening();
	}
};

Robot robotController;

void setup() {
	Serial.begin(9600);
	robotController.radio.begin();
	robotController.radio.openWritingPipe(writeAddress);
	robotController.radio.openReadingPipe(1, readAddress);
	robotController.radio.setPALevel(RF24_PA_MIN);
	robotController.radio.startListening();

  #if LEADER
  // Drive forward
  robotController.drive(255, 1000);
  robotController.sendData('0');

  // Wait 1 second
  delay(1000);

  // Turn left
  robotController.motors[LEFT].driveMotor(255);
  robotController.motors[RIGHT].driveMotor(-255);
  delay(1000);
  robotController.motors[LEFT].driveMotor(0);
  robotController.motors[RIGHT].driveMotor(0);
  robotController.sendData('1');

  // Wait 1 second
  delay(1000);

  // Turn right
  robotController.motors[LEFT].driveMotor(-255);
  robotController.motors[RIGHT].driveMotor(255);
  delay(1000);
  robotController.motors[LEFT].driveMotor(0);
  robotController.motors[RIGHT].driveMotor(0);
  robotController.sendData('2');
  #endif
}

void loop() {
	if(robotController.radio.available(0)){
		Serial.println("Message recieved: ");
    char message[1];
		robotController.radio.read(&message, sizeof(message));    
    Serial.println(message);
    Serial.println(sizeof(message));
    if(message[0] == SIG_FWD[0]){
      robotController.drive(255, 1000);
    }
    else if(message[0] == SIG_LEFT[0]){
      robotController.motors[LEFT].driveMotor(255);
      robotController.motors[RIGHT].driveMotor(-255);
      delay(1000);
      robotController.motors[LEFT].driveMotor(0);
      robotController.motors[RIGHT].driveMotor(0);
    }
    else if(message[0] == SIG_RIGHT[0]){
        robotController.motors[LEFT].driveMotor(-255);
        robotController.motors[RIGHT].driveMotor(255);
        delay(1000);
        robotController.motors[LEFT].driveMotor(0);
        robotController.motors[RIGHT].driveMotor(0);
    }
	}
}
