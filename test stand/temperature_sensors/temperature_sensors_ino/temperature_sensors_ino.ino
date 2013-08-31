void setup() {
  // initialize serial:
  Serial.begin(9600);
}

void loop() {
    Serial.print(-0.10752 * analogRead(0) + 79.6237);
    Serial.print('\t');
    Serial.print(-0.10752 * analogRead(1) + 79.6237);
    Serial.print('\t');
    Serial.print(-0.10752 * analogRead(2) + 79.6237);
    Serial.print('\n');
    delay(500);
}



