import processing.serial.*;
import java.util.Date;

Serial arduino;
PrintWriter output; 
PFont f;

float angleA = 0;
float angleB = 0;
float angleC = 0;

int x = 10, y = 20, w = 50, h = 25;

boolean running = false;
boolean initializationComplete = false;

void setup() {
  size(1200, 600);
  smooth();
  noLoop();
  
  output = createWriter("test stand data " + mydate(0) + ".txt"); 
  
  f = createFont("Arial", 16, true);
  
  frameRate(4);
  
  arduino = new Serial(this, Serial.list()[1], 9600);
  arduino.bufferUntil('\n');
}

double seconds = 0;
void draw() {
  if (initializationComplete) {
    if (running){
      background(100,255,100);
    } else {
      background(245);
    }
  } else {
    background(0);
  }
  
  textFont(f, 32);
  textAlign(CENTER);
  
  stroke(0);
  rect(x,y,w,h);
  fill(255);
  
  translate(width/6, height/2);
  fill(0);
  text (angleA+"psi", 0, 200);
  fill(255);
  stroke(0, 0, 0);
  ellipse(0, 0, height/2, height/2);
  strokeWeight(2);
  stroke(255, 0, 0) ;
  guage(height/4, angleA);

  translate(width/3, 0);
  fill(0);
  text (angleB+"\u00b0C", 0, 200);
  fill(255);
  stroke(0, 0, 0);
  ellipse(0, 0, height/2, height/2);
  stroke(255, 0, 0);
  guage(height/4, angleB);
  
  translate(width/3, 0);
  fill(0);
  text (angleC+"\u00b0C", 0, 200);
  stroke(0, 0, 0);
  ellipse(0, 0, height/2, height/2);
  stroke(255, 0, 0);
  guage(height/4, angleC);

  String values = seconds + "\t" + angleA + "\t" + angleB + "\t" + angleC;
  
  println(values);
  output.println(values);
  
  seconds += 0.1;
}

String mydate(int offset) {
  Date d = new Date();
  long timestamp = d.getTime() + (86400000 * offset);
  String date = new java.text.SimpleDateFormat("yyyy-MM-dd HH mm ss").format(timestamp);
  return date;
}

void mousePressed() {
  if(mousePressed){
    if(mouseX>x && mouseX <x+w && mouseY>y && mouseY <y+h){
      if(initializationComplete) arduino.write("run\n");
    }
   } 
}

void keyPressed() {
  output.flush(); // Writes the remaining data to the file
  output.close(); // Finishes the file
  exit(); // Stops the program
}

void serialEvent (Serial arduino)
{
  try { 
  String label = "";
  float value = 0;
  label = trim(arduino.readStringUntil(':')) ;
  value = float(trim(arduino.readStringUntil('\n')));
  if (label.equals("Pressure0:")) {
      angleA=(value/1000)*360;
  }
  angleB = 0;
  angleC = 0;
  
  } catch (Exception e) {
    String text = trim(arduino.readStringUntil('\n'));
    if(text.equals("Engine Starting")) {
      running = true;
    }
    if(text.equals("Engine Stopped")) {
      running = false;
    }
    if(text.indexOf("Initialization Complete!") != -1) {
      initializationComplete = true;
    }
  }
  redraw();
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
