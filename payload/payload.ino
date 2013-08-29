#include <SD.h>
#include <Wire.h> // for I2C communication

#define ledPin = 9;
#define tempPin = 0;
#define launchPin = 2;
#define chipSelectPin = 10;

boolean printedHead = false;
long previousMillis = 0; 
int ledState = LOW;
long flashInterval = 100;

// Accelerometer ADXL345
#define ACC 0x53    //ADXL345 ACC address
#define A_TO_READ (6)        //num of bytes we are going to read each time (two bytes for each axis)
// Gyroscope ITG3200 
#define GYRO 0x69 // gyro address, binary = 11101001 when AD0 is connected to Vcc (see schematics of your breakout board)
#define G_SMPLRT_DIV 0x15
#define G_DLPF_FS 0x16
#define G_INT_CFG 0x17
#define G_PWR_MGM 0x3E
#define G_TO_READ 8 // 2 bytes for each axis x, y, z

// offsets are chip specific. 
#define g_offx = 120;
#define g_offy = 20;
#define g_offz = 93;

char str[512]; 
 
#define ALTMODE; //comment out for barometer mode; default is altitude mode
#define ALTBASIS 18 //start altitude to calculate mean sea level pressure in meters
//this altitude must be known (or provided by GPS etc.)
 
#define ALT 0x60 // address specific to the MPL3115A1, value found in datasheet
 
float altsmooth = 0; //for exponential smoothing
byte IICdata[5] = {0,0,0,0,0}; //buffer for sensor data


void setup(){
  Wire.begin();
  Serial.begin(115200);
  initSD();
  initAlt();
  initAcc();
  initGyro();
}
 
void loop(){
  
  int alt[3];
  int acc[3];
  int gyro[4];
  getAltimeterData(alt);
  getAccelerometerData(acc);
  getGyroscopeData(gyro);
  
  sprintf(str, "%d,%d,%d,%d,%d,%d,%d,%d,%d", acc[0], acc[1], acc[2], gyro[0], gyro[1], gyro[2], gyro[3], alt[0], alt[1], alt[2]);  
  Serial.println(str);
}

void log(String str){
 Serial.println(str);
}

void flash(){
	 if(currentMillis - previousMillis > flashInterval) {
    previousMillis = currentMillis;  
    if (ledState == LOW)
      ledState = HIGH;
    else
      ledState = LOW;
    digitalWrite(ledPin, ledState);
  }
}
void initSD(){
  pinMode(ledPin, OUTPUT);
  pinMode(launchPin, INPUT);
  digitalWrite(launchPin, HIGH);
  
  Serial.print("Initializing SD card...");
  pinMode(chipSelectPin, OUTPUT);
  
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelectPin)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("Card initialized.");
}


void initAlt(){
  Serial.println("Altimeter Initialization...");
  if(IIC_Read(0x0C) != 196)
    Serial.println("altimeter i2c bad");
 
  writeTo(ALT, 0x2D,0); //write altitude offset=0 (because calculation below is based on offset=0)
  //calculate sea level pressure by averaging a few readings
  float buff[4];
  for (byte i=0;i<4;i++){
    writeTo(ALT, 0x26, 0b00111011); //bit 2 is one shot mode, bits 4-6 are 128x oversampling
    writeTo(ALT, 0x26, 0b00111001); //must clear oversampling (OST) bit, otherwise update will be once per second
    delay(550); //wait for sensor to read pressure (512ms in datasheet)
    IIC_ReadData(); //read sensor data
    buff[i] = Baro_Read(); //read pressure
  }
  float currpress=(buff[0]+buff[1]+buff[2]+buff[3])/4; //average over two seconds
  //calculate pressure at mean sea level based on a given altitude
  float seapress = currpress/pow(1-ALTBASIS*0.0000225577,5.255877);
  Serial.print("Temperature: ");
  Serial.print(IICdata[3]+(float)(IICdata[4]>>4)/16); Serial.println(" C");
 
  // This configuration option calibrates the sensor according to
  // the sea level pressure for the measurement location (2 Pa per LSB)
  writeTo(ALT, 0x14, (unsigned int)(seapress / 2)>>8);//IIC_Write(0x14, 0xC3); // BAR_IN_MSB (register 0x14):
  writeTo(ALT, 0x15, (unsigned int)(seapress / 2)&0xFF);//IIC_Write(0x15, 0xF3); // BAR_IN_LSB (register 0x15):
 
  //one reading seems to take 4ms (datasheet p.33);
  //oversampling 32x=130ms interval between readings seems to be best for 10Hz; slightly too slow
  //first bit is altitude mode (vs. barometer mode)
 
  //Altitude mode
  writeTo(ALT, 0x26, 0b10111011); //bit 2 is one shot mode //0xB9 = 0b10111001
  writeTo(ALT, 0x26, 0b10111001); //must clear oversampling (OST) bit, otherwise update will be once per second
  delay(550); //wait for measurement
  IIC_ReadData(); //
  altsmooth=Alt_Read();
  Serial.print("Altitude : "); Serial.println(altsmooth);
  Serial.println("Altimeter Initializated!");
}

void initAcc() {
  Serial.println("Accelerometer Initialization...");
  //Turning on the ADXL345
  writeTo(ACC, 0x2D, 0);      
  writeTo(ACC, 0x2D, 16);
  writeTo(ACC, 0x2D, 8);
  //by default the device is in +-2g range reading
  Serial.println("Accelerometer Initialized!");
}
 
 
 void initGyro()
{
  Serial.println("Gyroscope Initialization...");
  /*****************************************
  * ITG 3200
  * power management set to:
  * clock select = internal oscillator
  *     no reset, no sleep mode
  *   no standby mode
  * sample rate to = 125Hz
  * parameter to +/- 2000 degrees/sec
  * low pass filter = 5Hz
  * no interrupt
  ******************************************/
  writeTo(GYRO, G_PWR_MGM, 0x00);
  writeTo(GYRO, G_SMPLRT_DIV, 0x07); // EB, 50, 80, 7F, DE, 23, 20, FF
  writeTo(GYRO, G_DLPF_FS, 0x1E); // +/- 2000 dgrs/sec, 1KHz, 1E, 19
  writeTo(GYRO, G_INT_CFG, 0x00);
  Serial.println("Gyroscope Initialized...");
}

void getGyroscopeData(int * result)
{
  /**************************************
  Gyro ITG-3200 I2C
  registers:
  temp MSB = 1B, temp LSB = 1C
  x axis MSB = 1D, x axis LSB = 1E
  y axis MSB = 1F, y axis LSB = 20
  z axis MSB = 21, z axis LSB = 22
  *************************************/

  int regAddress = 0x1B;
  int temp, x, y, z;
  byte buff[G_TO_READ];
  
  readFrom(GYRO, regAddress, G_TO_READ, buff); //read the gyro data from the ITG3200
  
  result[0] = ((buff[2] << 8) | buff[3]) + g_offx;
  result[1] = ((buff[4] << 8) | buff[5]) + g_offy;
  result[2] = ((buff[6] << 8) | buff[7]) + g_offz;
  result[3] = (buff[0] << 8) | buff[1]; // temperature
  
}

void getAccelerometerData(int * result) {
  int regAddress = 0x32;    //first axis-acceleration-data register on the ADXL345
  byte buff[A_TO_READ];
  
  readFrom(ACC, regAddress, A_TO_READ, buff); //read the acceleration data from the ADXL345
  
  //each axis reading comes in 10 bit resolution, ie 2 bytes.  Least Significat Byte first!!
  //thus we are converting both bytes in to one int
  result[0] = (((int)buff[1]) << 8) | buff[0];   
  result[1] = (((int)buff[3])<< 8) | buff[2];
  result[2] = (((int)buff[5]) << 8) | buff[4];
}

void getAltimeterData(int * result){
  // This function reads the altitude (or barometer) and temperature registers, then prints their values
  // variables for the calculations
  int m_temp;
  float l_temp;
  float altbaro, temperature;
 
  //One shot mode at 0b10101011 is slightly too fast, but better than wasting sensor cycles that increase precision
  //one reading seems to take 4ms (datasheet p.33);
  //oversampling at 32x=130ms interval between readings seems to be optimal for 10Hz
  #ifdef ALTMODE //Altitude mode
    writeTo(ALT, 0x26, 0b10111011); //bit 2 is one shot mode //0xB9 = 0b10111001
    writeTo(ALT, 0x26, 0b10111001); //must clear oversampling (OST) bit, otherwise update will be once per second
  #else //Barometer mode
    writeTo(ALT, 0x26, 0b00111011); //bit 2 is one shot mode //0xB9 = 0b10111001
    writeTo(ALT, 0x26, 0b00111001); //must clear oversampling (OST) bit, otherwise update will be once per second
  #endif
 
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
 
  result[0] = altbaro;
  result[1] = altsmooth;
  result[2] = temperature;
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
  Wire.beginTransmission(ALT);
  Wire.write(regAddr); // Address of CTRL_REG1
  Wire.endTransmission(false); // Send data to I2C dev with option for a repeated start. Works in Arduino V1.0.1
  Wire.requestFrom(ALT, 1);
  return Wire.read();
}
 
void IIC_ReadData(){  //Read Altitude/Barometer and Temperature data (5 bytes)
  //This is faster than reading individual register, as the sensor automatically increments the register address,
  //so we just keep reading...
  byte i=0;
  Wire.beginTransmission(ALT);
  Wire.write(0x01); // Address of CTRL_REG1
  Wire.endTransmission(false);
  Wire.requestFrom(ALT,5); //read 5 bytes: 3 for altitude or pressure, 2 for temperature
  while(Wire.available()) IICdata[i++] = Wire.read();
}
 
 
 //reads num bytes starting from address register on ACC in to buff array
void readFrom(int DEVICE, byte address, int num, byte buff[]) {
  Wire.beginTransmission(DEVICE); //start transmission to ACC 
  Wire.write(address);        //sends address to read from
  Wire.endTransmission(); //end transmission
  
  Wire.beginTransmission(DEVICE); //start transmission to ACC
  Wire.requestFrom(DEVICE, num);    // request 6 bytes from ACC
  
  int i = 0;
  while(Wire.available())    //ACC may send less than requested (abnormal)
  { 
    buff[i] = Wire.read(); // receive a byte
    i++;
  }
  Wire.endTransmission(); //end transmission
}
 
void writeTo(int DEVICE, byte address, byte val) {
   Wire.beginTransmission(DEVICE); //start transmission to ACC 
   Wire.write(address);        // send register address
   Wire.write(val);        // send value to write
   Wire.endTransmission(); //end transmission
}
