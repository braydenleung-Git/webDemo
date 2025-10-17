extern const int trigPin,echoPin;
long duration;
float distanceCm;
float distanceInch;
bool toggleUltraSonics = false;


//define sound speed in cm/uS
#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701

void triggerUltraSonics(){
  if(toggleUltraSonics){
    // Clears the trigPin
    digitalWrite(trigPin, LOW);
    delayMicroseconds(5);
    // Sets the trigPin on HIGH state for 10 micro seconds
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    // Reads the echoPin, returns the sound wave travel time in microseconds
    duration = pulseIn(echoPin, HIGH);
    
    // Calculate the distance
    distanceCm = duration * SOUND_SPEED/2;
    
    // Convert to inches
    distanceInch = distanceCm * CM_TO_INCH;
    // Prints the distance in the Serial Monitor
    // Serial.print("Distance (cm): ");
    // Serial.println(distanceCm);
    // Serial.print("Distance (inch): ");
    // Serial.println(distanceInch);
  } else {
    distanceCm = 0;
    distanceInch = 0;
  }
}