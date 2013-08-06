#include <SD.h>

const int ledPin = 9;
const int tempPin = 0;
const int launchPin = 2;
const int chipSelectPin = 10;

boolean printedHead = false;
long previousMillis = 0; 
int ledState = LOW;
long interval = 100; 

void setup() {
 // Open serial communications and wait for port to open:
  Serial.begin(9600);
   while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

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
  Serial.println("card initialized.");
}

void loop() {
  unsigned long currentMillis = millis();
    
  String dataString = "";
  dataString += String(currentMillis) + ',' + String(analogRead(tempPin)) + ',' + String(digitalRead(launchPin));

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile && SD.exists("datalog.txt")) {
    if(!printedHead){
       dataString = "time, temp, launch";
      printedHead = true;
    }
    dataFile.println(dataString);
    dataFile.close();
    Serial.println(dataString);
    interval = 1000;
  }  
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
    interval = 100;
  }
  
 
  if(currentMillis - previousMillis > interval) {
    previousMillis = currentMillis;  
    if (ledState == LOW)
      ledState = HIGH;
    else
      ledState = LOW;
    digitalWrite(ledPin, ledState);
  }
  
  delay(1);
}
