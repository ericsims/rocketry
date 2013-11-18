const int ledPin =  13;
const int fuelPin = 11;
const int airPin =  10;
const int oxyPin =  9;

int ledState = LOW;
long ledPreviousMillis = 0;
long blinkInterval = 500;

long engineStartTime = 0;
boolean engineRunning = false;
long engineBurnTime = 10000;
long oxyDelayTime = 1000;
long fuelDelayTime = 0;

String inputString = "";
boolean stringComplete = false;
void setup() {
  Serial.begin(9600);

  pinMode(ledPin, OUTPUT);
  pinMode(fuelPin, OUTPUT);
  pinMode(airPin, OUTPUT);
  pinMode(oxyPin, OUTPUT);
  
  inputString.reserve(200);
  
 Serial.println("Initialization Complete!");
}

void loop()
{
  unsigned long currentMillis = millis();
 
  if(engineRunning) {
    ledState = HIGH;
  } else {
    if(currentMillis - ledPreviousMillis > blinkInterval) {
      ledPreviousMillis = currentMillis;
      if (ledState == LOW)
        ledState = HIGH;
      else
        ledState = LOW;
    }
  }
  digitalWrite(ledPin, ledState);
  
  
  if (stringComplete) {
    Serial.print(inputString);
    if(inputString == "run\n"){
      Serial.println("Engine Starting");
      engineStartTime = currentMillis;
      engineRunning = true;
    } else if (inputString == "stop\n"){
      engineRunning = false;
    }
    inputString = "";
    stringComplete = false;
  }
  
  if(engineRunning && engineStartTime + engineBurnTime > currentMillis){
    if(engineStartTime + fuelDelayTime < currentMillis)
      digitalWrite(fuelPin, HIGH);
    if(engineStartTime + oxyDelayTime < currentMillis)
      digitalWrite(oxyPin, HIGH);
  } else {
    if(engineRunning){
      engineRunning = false;
      Serial.println("Engine Stopped");
    }
    digitalWrite(fuelPin, LOW);
    digitalWrite(oxyPin, LOW);
  }
  
  Serial.print("Voltage:");
  Serial.println(analogRead(0));
  
  Serial.print("Pressure0:");
  Serial.println(9.9206 * (analogRead(1) - 125));
  
  Serial.print("Temp0:");
  Serial.println(analogRead(2));
  
  delay(100);
}

void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    inputString += inChar;
    if (inChar == '\n') {
      stringComplete = true;
    }
  }
}
