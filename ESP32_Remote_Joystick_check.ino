void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 ADC Test OK");
  pinMode(15, OUTPUT);
  pinMode(21, OUTPUT);
}

void loop() {
  delay(1000);
  digitalWrite(21, HIGH);
  delay(500);
  digitalWrite(15, HIGH);
  int a = analogRead(34);
  int b = analogRead(35);
  int c = analogRead(36);
  int d = analogRead(39);
  int isON = digitalRead(13);

  Serial.print(a);
  Serial.print("  ");
  Serial.print(b);
  Serial.print("  ");
  Serial.print(c);
  Serial.print("  ");
  Serial.println(d);
  // if (Serial.available()){
  //   Serial.print(isON);
  //   char on = Serial.read();
      // pinMode(13, INPUT);
  //   if (on=='1') digitalWrite(13, HIGH);
  //   else digitalWrite(13, LOW);
  // }

  delay(1000);

  digitalWrite(21,LOW);
  digitalWrite(15,LOW);
 delay(1000);




}
