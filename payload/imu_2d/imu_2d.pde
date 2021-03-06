import processing.serial.*;

float pos = 10;
Serial com3;
int lf = 10;      // ASCII linefeed
String inString = "";  // Input string from serial port

void setup() {  // setup() runs once
  println(Serial.list());
  com3 = new Serial(this, "COM3", 115200); //Serial.list()[1]
  com3.bufferUntil(lf);
  size(200, 200);
  frameRate(60);
}
 
void draw() {  // draw() loops forever, until stopped
  background(204);
  int[] sensorVals = new int[10];

  if(inString.indexOf(",") != -1){
      String[] val = splitTokens(inString, ",");
      line(width/2, height, width * ((Integer.parseInt(val[0]) / 280f) + .5), height/2);
      line(width/2, height, width * ((Integer.parseInt(val[1]) / 280f) + .5), height/2);
      line( 0, height - (Float.parseFloat(val[8])), width, height - (Float.parseFloat(val[8])) );
  }

  println(inString);

}

void serialEvent(Serial p) {
  inString = (com3.readString());
}
