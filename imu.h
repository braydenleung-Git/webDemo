#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h> //Library that communicate to I2C-device 


Adafruit_MPU6050 sensor;
float a_x, a_y, a_z, g_x,g_y,g_z, temp_c;
extern bool toggleSerial;

void setupIMU(){
  // Try to initialize!
  if (!sensor.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");
  /*
    Other range options: (+-n G,G = G-force)(normal people can sustain up to 4-6G before passing out!)
    MPU6050_RANGE_2_G (+-2G)
    MPU6050_RANGE_4_G (+-4G)
    MPU6050_RANGE_16_G (+-16G)
  */
  sensor.setAccelerometerRange(MPU6050_RANGE_8_G);

  /*
    Other range options: (+-n deg/s)
    MPU6050_RANGE_250_DEG 
    MPU6050_RANGE_1000_DEG
    MPU6050_RANGE_2000_DEG 
  */
  sensor.setGyroRange(MPU6050_RANGE_500_DEG);

  /*
    Other filter options: (+-n Hz)(lower = smoother/less fluctuation)
    MPU6050_BAND_260_HZ
    MPU6050_BAND_184_HZ
    MPU6050_BAND_94_HZ
    MPU6050_BAND_44_HZ
    MPU6050_BAND_21_HZ
    MPU6050_BAND_10_HZ
    MPU6050_BAND_5_HZ
  */
  sensor.setFilterBandwidth(MPU6050_BAND_5_HZ);
}

void triggerIMU(){
  if(toggleSerial){
    /* Get new sensor events with the readings */
    sensors_event_t a, g, temp;
    sensor.getEvent(&a, &g, &temp);
    //m/s^2
    a_x = a.acceleration.x;
    a_y = a.acceleration.y;
    a_z = a.acceleration.z;
    //rad/s
    g_x = g.gyro.x;
    g_y = g.gyro.y;
    g_z = g.gyro.z;
    //deg(C)
    temp_c = temp.temperature;
  }
  
}

