#include <string.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd1(0x20, 16, 2);
LiquidCrystal_I2C lcd2(0x27, 16, 2);

int rpin = 7;
int prev = 0;
int i;

char param[60];
String paraml1;
String paraml2;

void setup() {

  pinMode(rpin, OUTPUT);

  Serial.begin(115200);

  lcd1.begin();
  lcd2.begin();

  lcd1.backlight();
  lcd2.backlight();
  
  lcd1.setCursor(0,0);
  lcd1.print("     OORJA    ");
  lcd1.setCursor(0,1);
  lcd1.print("Power Monitoring");
  
  lcd2.setCursor(0,0);
  lcd2.print("   Connecting");
  lcd2.setCursor(0,1);
  lcd2.print(" . . . . . . . . ");

}

void loop() {

  if (Serial.available() > 0){
    i = 0;
    while (Serial.available()){

      char c = Serial.read();
      param[i] = c;
      i++;

    }

    char *token;
    token = strtok(param, ";");
    paraml1 = token;
    token = strtok(NULL, ";");
    paraml2 = token;

    lcd2.clear();

    lcd2.setCursor(0,0);
    lcd2.print(paraml1);
    lcd2.setCursor(0,1);  
    lcd2.print(paraml2);

    paraml1 = "";
    paraml2 = "";

  }
    
  
  if (digitalRead(rpin) == LOW && prev == 1) {
    delay(500);
    prev = 0;
    Serial.write("OFF");
  }

  if (digitalRead(rpin) == HIGH && prev == 0) {
    delay(500);
    prev = 1;
    Serial.write("ON");
  }

}
