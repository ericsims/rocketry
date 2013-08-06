/*
 
MPL3115A2 Altitude Sensor One Shot Mode Example
 Henry Lahr, 2013-02-27
 loosely based on: https://github.com/sparkfun/MPL3115A2_Breakout/blob/master/firmware/mpl3115a2/mpl3115a2.ino
 There is no warranty for this code; feel free to use this code for your projects.
 Hardware Connections: VCC = 3.3V; SDA = A4; SCL = A5; INT pins not connected
 Usage:
 - Serial terminal at 115200bps
 - Prints altitude in meters or temperature in degrees C, depending on whether ALTMODE is defined
 */
 
#include <Wire.h> // for I2C communication
 
#define ALTMODE; //comment out for barometer mode; default is altitude mode
#define ALTBASIS 18 //start altitude to calculate mean sea level pressure in meters
//this altitude must be known (or provided by GPS etc.)
 
const int ALTADDRESS = 0x60; // address specific to the MPL3115A1, value found in datasheet
 
float altsmooth = 0; //for exponential smoothing
byte IICdata[5] = {0,0,0,0,0}; //buffer for sensor data
 
void setup(){
  Wire.begin(); // join i2c bus
  Serial.begin(115200); // start serial for output
  Serial.println("Setup");
  if(IIC_Read(0x0C) == 196); //checks whether sensor is readable (who_am_i bit)
  else Serial.println("i2c bad");
 
  IIC_Write(0x2D,0); //write altitude offset=0 (because calculation below is based on offset=0)
  //calculate sea level pressure by averaging a few readings
  Serial.println("Pressure calibration...");
  float buff[4];
  for (byte i=0;i<4;i++){
    IIC_Write(0x26, 0b00111011); //bit 2 is one shot mode, bits 4-6 are 128x oversampling
    IIC_Write(0x26, 0b00111001); //must clear oversampling (OST) bit, otherwise update will be once per second
    delay(550); //wait for sensor to read pressure (512ms in datasheet)
    IIC_ReadData(); //read sensor data
    buff[i] = Baro_Read(); //read pressure
    Serial.println(buff[i]);
  }
  float currpress=(buff[0]+buff[1]+buff[2]+buff[3])/4; //average over two seconds
 
  Serial.print("Current pressure: "); Serial.print(currpress); Serial.println(" Pa");
  //calculate pressure at mean sea level based on a given altitude
  float seapress = currpress/pow(1-ALTBASIS*0.0000225577,5.255877);
  Serial.print("Sea level pressure: "); Serial.print(seapress); Serial.println(" Pa");
  Serial.print("Temperature: ");
  Serial.print(IICdata[3]+(float)(IICdata[4]>>4)/16); Serial.println(" C");
 
  // This configuration option calibrates the sensor according to
  // the sea level pressure for the measurement location (2 Pa per LSB)
  IIC_Write(0x14, (unsigned int)(seapress / 2)>>8);//IIC_Write(0x14, 0xC3); // BAR_IN_MSB (register 0x14):
  IIC_Write(0x15, (unsigned int)(seapress / 2)&0xFF);//IIC_Write(0x15, 0xF3); // BAR_IN_LSB (register 0x15):
 
  //one reading seems to take 4ms (datasheet p.33);
  //oversampling 32x=130ms interval between readings seems to be best for 10Hz; slightly too slow
  //first bit is altitude mode (vs. barometer mode)
 
  //Altitude mode
  IIC_Write(0x26, 0b10111011); //bit 2 is one shot mode //0xB9 = 0b10111001
  IIC_Write(0x26, 0b10111001); //must clear oversampling (OST) bit, otherwise update will be once per second
  delay(550); //wait for measurement
  IIC_ReadData(); //
  altsmooth=Alt_Read();
  Serial.print("Altitude now: "); Serial.println(altsmooth);
  Serial.println("Done.");
}
 
void loop(){
  sensor_read_data();
  // your code here
}
 
void sensor_read_data(){
  // This function reads the altitude (or barometer) and temperature registers, then prints their values
  // variables for the calculations
  int m_temp;
  float l_temp;
  float altbaro, temperature;
 
  //One shot mode at 0b10101011 is slightly too fast, but better than wasting sensor cycles that increase precision
  //one reading seems to take 4ms (datasheet p.33);
  //oversampling at 32x=130ms interval between readings seems to be optimal for 10Hz
  #ifdef ALTMODE //Altitude mode
    IIC_Write(0x26, 0b10111011); //bit 2 is one shot mode //0xB9 = 0b10111001
    IIC_Write(0x26, 0b10111001); //must clear oversampling (OST) bit, otherwise update will be once per second
  #else //Barometer mode
    IIC_Write(0x26, 0b00111011); //bit 2 is one shot mode //0xB9 = 0b10111001
    IIC_Write(0x26, 0b00111001); //must clear oversampling (OST) bit, otherwise update will be once per second
  #endif
  delay(100); //read with 10Hz; drop this if calling from an outer loop
 
  IIC_ReadData(); //reads registers from the sensor
  m_temp = IICdata[3]; //temperature, degrees
  l_temp = (float)(IICdata[4]>>4)/16.0; //temperature, fraction of a degree
  temperature = (float)(m_temp + l_temp);
 
  #ifdef ALTMODE //converts byte data into float; change function to Alt_Read() or Baro_Read()
    altbaro = Alt_Read();
  #else
    altbaro = Baro_Read();
  #endif
 
  altsmooth=(altsmooth*3+altbaro)/4; //exponential smoothing to get a smooth time series
 
  Serial.print(altbaro); // in meters or Pascal
  Serial.print(",");
  Serial.print(altsmooth); // exponentially smoothed
  Serial.print(",");
  Serial.println(temperature); // in degrees C
}
 
float Baro_Read(){
  //this function takes values from the read buffer and converts them to pressure units
  unsigned long m_altitude = IICdata[0];
  unsigned long c_altitude = IICdata[1];
  float l_altitude = (float)(IICdata[2]>>4)/4; //dividing by 4, since two lowest bits are fractional value
  return((float)(m_altitude<<10 | c_altitude<<2)+l_altitude); //shifting 2 to the left to make room for LSB
}
 
float Alt_Read(){
  //Reads altitude data (if CTRL_REG1 is set to altitude mode)
  int m_altitude = IICdata[0];
  int c_altitude = IICdata[1];
  float l_altitude = (float)(IICdata[2]>>4)/16;
  return((float)((m_altitude << 8)|c_altitude) + l_altitude);
}
 
byte IIC_Read(byte regAddr){
  // This function reads one byte over I2C
  Wire.beginTransmission(ALTADDRESS);
  Wire.write(regAddr); // Address of CTRL_REG1
  Wire.endTransmission(false); // Send data to I2C dev with option for a repeated start. Works in Arduino V1.0.1
  Wire.requestFrom(ALTADDRESS, 1);
  return Wire.read();
}
 
void IIC_ReadData(){  //Read Altitude/Barometer and Temperature data (5 bytes)
  //This is faster than reading individual register, as the sensor automatically increments the register address,
  //so we just keep reading...
  byte i=0;
  Wire.beginTransmission(ALTADDRESS);
  Wire.write(0x01); // Address of CTRL_REG1
  Wire.endTransmission(false);
  Wire.requestFrom(ALTADDRESS,5); //read 5 bytes: 3 for altitude or pressure, 2 for temperature
  while(Wire.available()) IICdata[i++] = Wire.read();
}
 
void IIC_Write(byte regAddr, byte value){
  // This function writes one byto over I2C
  Wire.beginTransmission(ALTADDRESS);
  Wire.write(regAddr);
  Wire.write(value);
  Wire.endTransmission(true);
}
