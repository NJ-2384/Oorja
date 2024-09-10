String rstate;

int rpin = 12;
float prevvol = 0.00;
float prevcurr = 0.00;
float prevpower = 0.00;
int adcvalue= 0;
float ecurrent = 0;
float tolerance = 0.5;


void setup() {

  Serial.begin(115200);
  pinMode(rpin, OUTPUT);

}

void loop() {

  if (Serial.available() != 0) {
    rstate = Serial.readString();

    if (rstate == "OFF") {
      digitalWrite(rpin, LOW);
  }

    if (rstate == "ON") {
      digitalWrite(rpin, HIGH);
  }

  }


  int sensorValue = analogRead(A1);
  float vol = (sensorValue * 25.0) / 1023.0;
  adcvalue = analogRead(A0);
  ecurrent = ((((adcvalue / 1024.0) * 5000) - 2500) / 185);
  float power = (vol * ecurrent);

  if (vol != prevvol || ecurrent != prevcurr || power != prevpower) {

    if (power == 0) {

      Serial.print(" Device;    Disconnected");
      prevvol = vol;
      prevcurr = ecurrent;
      prevpower = power;

    }

    else {

      Serial.print("  V=");
      Serial.print(vol);
      Serial.print("v");
      prevvol = vol;
      Serial.print(" I=");
      Serial.print(ecurrent);
      Serial.print("A");
      prevcurr = ecurrent;
      Serial.print(";   P=");
      Serial.print(power);
      Serial.print("Watt");
      prevpower = power;

    }

    delay(200);

    }
  
}
