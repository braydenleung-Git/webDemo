/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-hc-sr04-ultrasonic-arduino/
  Complete project details at https://randomnerdtutorials.com/esp32-mpu-6050-accelerometer-gyroscope-arduino/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/
/*
  This code has been adapt from the source above, but majority of the code was original to braydenleung-Git.
  Important note: There are more than 1 ways to recreate what I did, just because you got another idea does not mean it is wrong, unless you started to see magic smoke.
*/

/*library required for this code to work 
- Ardunio_JSON by Ardunio
- HCSR04 ultrasonic sensor by gamegine
- Adafruit MPU6050 by Adafruit (click yes to install dependency)
*/
#include "web.h"


const int trigPin = 5; //GPIO 5
const int echoPin = 18;//GPIO 18


void setup(){
  Serial.begin(115200); // Starts the serial communication
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  setupWebServer();
  setupIMU();
  
}

void loop(){
  dnsServer.processNextRequest();
  server.handleClient();
  triggerUltraSonics();
  triggerIMU();
  Serial.println("");
  delay(1000);
}

