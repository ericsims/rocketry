import processing.serial.*;

float pos = 10;
Serial com3;
int lf = 10;      // ASCII linefeed
String inString = "";  // Input string from serial port

void setup() {  // setup() runs once
  println(Serial.list());
  com3 = new Serial(this, "COM3", 115200); //Serial.list()[1]
  com3.bufferUntil(lf);
  size(640, 360, P3D);
  noStroke();
  fill(204);
  frameRate(60);
}
 
void draw() {  // draw() loops forever, until stopped
  background(0);
  lights();
  float fov = PI/3.0; 
  float cameraZ = (height/2.0) / tan(fov/2.0); 
  perspective(fov, float(width)/float(height), cameraZ/2.0, cameraZ*2.0); 
  translate(width/2, height/2, 0);
  rotateY(PI/3); 
  box(160); 
  
  if(inString.indexOf(",") != -1){
      String[] val = splitTokens(inString, ",");
      rotateX((Integer.parseInt(val[0]) / 280f) * PI);
      line(width/2, height, width * ((Integer.parseInt(val[1]) / 280f) + .5), height/2);
      line( 0, height - (Float.parseFloat(val[8])), width, height - (Float.parseFloat(val[8])) );
  }

  println(inString);

}

void serialEvent(Serial p) {
  inString = (com3.readString());
}
