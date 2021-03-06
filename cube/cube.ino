#include "Wire.h"

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>


IPAddress    apIP(42, 42, 42, 42);  //ip of cube is 42.42.42.42
ESP8266WebServer server(80); // run a webserver on port 80
const char *ssid = "The Privacy Cube";
const char *password = "facebook";

const uint8_t MPU_addr=0x68; // I2C address of the MPU-6050

const float MPU_GYRO_250_SCALE = 131.0;
const float MPU_GYRO_500_SCALE = 65.5;
const float MPU_GYRO_1000_SCALE = 32.8;
const float MPU_GYRO_2000_SCALE = 16.4;
const float MPU_ACCL_2_SCALE = 16384.0;
const float MPU_ACCL_4_SCALE = 8192.0;
const float MPU_ACCL_8_SCALE = 4096.0;
const float MPU_ACCL_16_SCALE = 2048.0;

const float ACCL_THRESHOLD = 0.10;
 
struct rawdata {
  int16_t AcX;
  int16_t AcY;
  int16_t AcZ;
  int16_t Tmp;
  int16_t GyX;
  int16_t GyY;
  int16_t GyZ;
};
 
struct scaleddata{
  float AcX;
  float AcY;
  float AcZ;
  float Tmp;
  float GyX;
  float GyY;
  float GyZ;
};

const String webcontent = "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><title>Privacy Cube</title><style>body {margin: 40px;background-color: white;}#poster {display: grid;grid-gap: 20px;grid-template-columns: 1fr 1fr [last];}.box {background-color: black;border-radius: 5px;padding: 10px;text-align: center;}#active_mode_title {color: white;}.header {background-color: rgb(66, 202, 39);text-align: center;grid-column: 1/ last;}.footer {background-color: rgb(150, 243, 141);text-align: center;grid-column: 1/ last;}.zen {background-image: url(http://localhost/image-zen.png);background-position: center;background-repeat: no-repeat;background-size: contain;}.party {background-image: url(http://localhost/image-party.png);background-position: center;background-repeat: no-repeat;background-size: contain;}.party-hard {background-image: url(http://localhost/image-partyhard.png);background-position: center;background-repeat: no-repeat;background-size: contain;}.focus {background-image: url(http://localhost/image-focus.png);background-position: center;background-repeat: no-repeat;background-size: contain;}.relax {background-image: url(http://localhost/image-relax.png);background-position: center;background-repeat: no-repeat;background-size: contain;}.initial {background-image: url(http://localhost/image-initial.png);background-position: center;background-repeat: no-repeat;background-size: contain;}.size {min-height: 500px;}</style></head><body><div class=\"box header\"><h1>Active Side of the Privacy Cube</h1></div><p></p><div class=\"box\"><h1 id=\"active_mode_title\">Zen mode</h1></div><div id=\"active_mode\" class=\"box zen size\"></div><div class=\"box\"></div><p></p><div class=\"box footer\"><p>Created by (CIDs): gsanna, renee, wiebkem, wilfalk, urbanek </p></div><script>function changeContent (side) {console.log(\"set side to: \", side);var active_mode_title = document.getElementById(\"active_mode_title\");var active_mode = document.getElementById(\"active_mode\");switch(side) {case 1:active_mode_title.innerHTML= \"Zen mode\";active_mode.className= \"box zen size\";break;case 2:active_mode_title.innerHTML= \"Focus mode\";active_mode.className= \"box focus size\";break;case 3:active_mode_title.innerHTML= \"Party mode\";active_mode.className= \"box party size\";break;case 4:active_mode_title.innerHTML= \"Party-hard mode\";active_mode.className= \"box party-hard size\";break;case 5:active_mode_title.innerHTML= \"Relax mode\";active_mode.className= \"box relax size\";break;case 6:active_mode_title.innerHTML= \"Initial mode\";active_mode.className= \"box initial size\";break;}}function directionReceived () {changeContent(+this.responseText);}function getDirection () {oReq.open(\"GET\", \"/dir\");oReq.send();window.setTimeout(getDirection, 700);}var oReq = new XMLHttpRequest();oReq.addEventListener(\"load\", directionReceived);getDirection();</script></body></html>";

bool checkI2c(byte addr);
void mpu6050Begin(byte addr);
rawdata mpu6050Read(byte addr, bool Debug);
void setMPU6050scales(byte addr,uint8_t Gyro,uint8_t Accl);
void getMPU6050scales(byte addr,uint8_t &Gyro,uint8_t &Accl);
scaleddata convertRawToScaled(byte addr, rawdata data_in,bool Debug);
int direction = 1;
 
void setup() {
  Wire.begin(4,5);
  Serial.begin(115200);

  mpu6050Begin(MPU_addr);

  //set-up the custom IP address
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));   // subnet FF FF FF 00  
  
  /* You can remove the password parameter if you want the AP to be open. */
  WiFi.softAP(ssid, password);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  server.on ( "/", serveInterface );
  server.on ( "/dir", handleDirection );

  server.begin();
  Serial.println("HTTP server started");
}

void serveInterface(){
  server.send(200, "text/html", webcontent);
}

void handleDirection(){
  rawdata next_sample;
  setMPU6050scales(MPU_addr,0b00000000,0b00010000);
  next_sample = mpu6050Read(MPU_addr, true);
  convertRawToScaled(MPU_addr, next_sample,true);
  server.send(200, "text/plain", (String) direction);
}
 
void loop() {
  server.handleClient();
}

void mpu6050Begin(byte addr){
  // This function initializes the MPU-6050 IMU Sensor
  // It verifys the address is correct and wakes up the
  // MPU.
  if (checkI2c(addr)){
    Wire.beginTransmission(MPU_addr);
    Wire.write(0x6B); // PWR_MGMT_1 register
    Wire.write(0); // set to zero (wakes up the MPU-6050)
    Wire.endTransmission(true);
    delay(30); // Ensure gyro has enough time to power up
  }
}
 
bool checkI2c(byte addr){
  // We are using the return value of
  // the Write.endTransmisstion to see if
  // a device did acknowledge to the address.
  Serial.println(" ");
  Wire.beginTransmission(addr);

  if (Wire.endTransmission() == 0){
    Serial.print(" Device Found at 0x");
    Serial.println(addr,HEX);
    return true;
  } else {
    Serial.print(" No Device Found at 0x");
    Serial.println(addr,HEX);
    return false;
  }
}
 

rawdata mpu6050Read(byte addr, bool Debug){
  // This function reads the raw 16-bit data values from
  // the MPU-6050
  rawdata values;
   
  Wire.beginTransmission(addr);
  Wire.write(0x3B); // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(addr,14,true); // request a total of 14 registers
  values.AcX=Wire.read()<<8|Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
  values.AcY=Wire.read()<<8|Wire.read(); // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  values.AcZ=Wire.read()<<8|Wire.read(); // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
  values.Tmp=Wire.read()<<8|Wire.read(); // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
  values.GyX=Wire.read()<<8|Wire.read(); // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
  values.GyY=Wire.read()<<8|Wire.read(); // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
  values.GyZ=Wire.read()<<8|Wire.read(); // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)

  if(Debug){
    //Serial.print(" GyX = "); Serial.print(values.GyX);
    //Serial.print(" | GyY = "); Serial.print(values.GyY);
    //Serial.print(" | GyZ = "); Serial.print(values.GyZ);
    //Serial.print(" | Tmp = "); Serial.print(values.Tmp);
    Serial.print(" | AcX = "); Serial.print(values.AcX);
    Serial.print(" | AcY = "); Serial.print(values.AcY);
    Serial.print(" | AcZ = "); Serial.println(values.AcZ);
  }
   
  return values;
}
 
void setMPU6050scales(byte addr,uint8_t Gyro,uint8_t Accl){
  Wire.beginTransmission(addr);
  Wire.write(0x1B); // write to register starting at 0x1B
  Wire.write(Gyro); // Self Tests Off and set Gyro FS to 250
  Wire.write(Accl); // Self Tests Off and set Accl FS to 8g
  Wire.endTransmission(true);
}
 
void getMPU6050scales(byte addr,uint8_t &Gyro,uint8_t &Accl){
  Wire.beginTransmission(addr);
  Wire.write(0x1B); // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(addr,2,true); // request a total of 14 registers
  Gyro = (Wire.read()&(bit(3)|bit(4)))>>3;
  Accl = (Wire.read()&(bit(3)|bit(4)))>>3;
}

scaleddata convertRawToScaled(byte addr, rawdata data_in, bool Debug){
   
  scaleddata values;
  float scale_value = 0.0;
  byte Gyro, Accl;
   
  getMPU6050scales(MPU_addr, Gyro, Accl);
   
  if(Debug){
    Serial.print("Gyro Full-Scale = ");
  }
   
  switch (Gyro){
  case 0:
    scale_value = MPU_GYRO_250_SCALE;
    if(Debug){
      Serial.println("±250 °/s");
    }
    break;
  case 1:
    scale_value = MPU_GYRO_500_SCALE;
    if(Debug){
      Serial.println("±500 °/s");
    }
    break;
  case 2:
    scale_value = MPU_GYRO_1000_SCALE;
    if(Debug){
      Serial.println("±1000 °/s");
    }
    break;
  case 3:
    scale_value = MPU_GYRO_2000_SCALE;
    if(Debug){
      Serial.println("±2000 °/s");
    }
    break;
  default:
    break;
  }
   
  values.GyX = (float) data_in.GyX / scale_value;
  values.GyY = (float) data_in.GyY / scale_value;
  values.GyZ = (float) data_in.GyZ / scale_value;
   
  scale_value = 0.0;
  if(Debug){
    Serial.print("Accl Full-Scale = ");
  }
  switch (Accl){
  case 0:
    scale_value = MPU_ACCL_2_SCALE;
    if(Debug){
      Serial.println("±2 g");
    }
    break;
  case 1:
    scale_value = MPU_ACCL_4_SCALE;
    if(Debug){
      Serial.println("±4 g");
    }
    break;
  case 2:
    scale_value = MPU_ACCL_8_SCALE;
    if(Debug){
      Serial.println("±8 g");
    }
    break;
  case 3:
    scale_value = MPU_ACCL_16_SCALE;
    if(Debug){
      Serial.println("±16 g");
    }
    break;
  default:
    break;
  }
  values.AcX = (float) data_in.AcX / scale_value;
  values.AcY = (float) data_in.AcY / scale_value;
  values.AcZ = (float) data_in.AcZ / scale_value;
   
    
   
  values.Tmp = (float) data_in.Tmp / 340.0 + 36.53;
   
  if(Debug){
    Serial.print(" GyX = "); Serial.print(values.GyX);
    Serial.print(" °/s| GyY = "); Serial.print(values.GyY);
    Serial.print(" °/s| GyZ = "); Serial.print(values.GyZ);
    Serial.print(" °/s| Tmp = "); Serial.print(values.Tmp);
    Serial.print(" °C| AcX = "); Serial.print(values.AcX);
    Serial.print(" g| AcY = "); Serial.print(values.AcY);
    Serial.print(" g| AcZ = "); Serial.print(values.AcZ);Serial.println(" g");
  }

   valuesToDirection(values);
  return values;
}

bool reasonablyClose(float value, float target) {
  return (value - ACCL_THRESHOLD < target) && (value + ACCL_THRESHOLD > target);
}

void valuesToDirection(scaleddata val) {
  //X value: sides 2 and 5
  if (reasonablyClose(val.AcX, -1)) {
    direction = 5; 
  }
  if (reasonablyClose(val.AcX, 1)) {
    direction = 2; 
  }

  //Y value: sides 3 and 4
  if (reasonablyClose(val.AcY, -1)) {
    direction = 4; 
  }
  if (reasonablyClose(val.AcY, 1)) {
    direction = 3; 
  }

  //Z value: sides 1 and 6
  if (reasonablyClose(val.AcZ, -1)) {
    direction = 1; 
  }
  if (reasonablyClose(val.AcZ, 1)) {
    direction = 6; 
  }

  if (true) {
    Serial.print("Direction: ");
    Serial.print(direction);
    Serial.print("\n");
  }

  return;
}



