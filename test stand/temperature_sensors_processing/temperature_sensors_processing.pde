import processing.serial.*;

Serial arduino;
PrintWriter output; 

float angleX = 0 ;
float angleY = 0 ;
float angleZ = 0 ;

void setup() {
  size(1200, 600) ;
  
  arduino = new Serial(this, Serial.list()[0], 9600);
  arduino.bufferUntil('\n');
  
  output = createWriter("temp.txt"); 
  
  frameRate(4);
}

double seconds = 0;
void draw() {
  background(255);
  translate(width/6, height/2);

  stroke(0, 0, 0);
  ellipse(0, 0, height/2, height/2);
  strokeWeight(2);
  stroke(255, 0, 0) ;
  guage(height/4, angleX);

  translate(width/3, 0);
  stroke(0, 0, 0);
  ellipse(0, 0, height/2, height/2);
  stroke(255, 0, 0);
  guage(height/4, angleY);
  
  translate(width/3, 0);
  stroke(0, 0, 0);
  ellipse(0, 0, height/2, height/2);
  stroke(255, 0, 0);
  guage(height/4, angleZ);

  String values = seconds + "\t" + angleX + "\t" + angleY + "\t" + angleZ;
  
  println(values);
  output.println(values);
  
  seconds += 0.25;
}

void keyPressed() {
  output.flush(); // Writes the remaining data to the file
  output.close(); // Finishes the file
  exit(); // Stops the program
}

void serialEvent (Serial arduino)
{
  //get the ASCII strings:
  angleX = float(trim(arduino.readStringUntil('\t')));
  angleY = float(trim(arduino.readStringUntil('\t')));
  angleZ = float(trim(arduino.readStringUntil('\n')));
}

void guage(float r, float angle) {
  // Conver from Degree -> Rad
  angle = (angle-180)*(PI/180);
  // Convert Polar -> Cartesian
  float x = r * cos(angle);
  float y = r * sin(angle);

  arrow(0, 0, (int)x, (int)y);
}

void arrow(int x1, int y1, int x2, int y2) {
  line(x1, y1, x2, y2);
  pushMatrix();
  translate(x2, y2);
  float a = atan2(x1-x2, y2-y1);
  rotate(a);
  line(0, 0, -10, -10);
  line(0, 0, 10, -10);
  popMatrix();
} 

