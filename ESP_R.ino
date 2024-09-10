//IO Pins
#define btn 4
#define debug_btn 0
#define debug_led 2
#define led_wifi 17
#define led_server 18

//Virtual IO Pins
#define vpin_info V0
#define vpin_current V1
#define vpin_voltage V2
#define vpin_power V3
#define vpin_relay V4

//Blynk Auth
#define BLYNK_TEMPLATE_ID "TMPL3LSVIFBUN"
#define BLYNK_TEMPLATE_NAME "Oorja Power Monitoring System"
#define BLYNK_AUTH_TOKEN "Yn6PKSG9E39eO3DJLTCsv-XpxmhMrXuW"


//Libraries
#include <WiFi.h>
#include <WiFiClient.h>
#include "WiFiProv.h"
#include <Preferences.h>
#include <BlynkSimpleEsp32.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>


//Wi-Fi Provisioning Variables
const char * pop = "Esp_RX32";
const char * service_name = "PROV_espRX";
const char * service_key = NULL;
bool reset_provisioned = false;

//Blynk Variables
char auth[] = BLYNK_AUTH_TOKEN;

//LCD Variables
char l1[16];
char l2[16];
char l3[16];
char l4[16];

bool btn_state;
bool server_flag;
bool debug;


//Instances
LiquidCrystal_I2C lcd1(0x27, 16, 2);
LiquidCrystal_I2C lcd2(0x20, 16, 2);
Preferences pref;
BlynkTimer timer;


//Wi-Fi Provisioning
void SysProvEvent(arduino_event_t *sys_event) {
    switch (sys_event->event_id) {

      case ARDUINO_EVENT_PROV_START:
        if (debug) Serial.println("No previous networks found\nInitialising Wi-Fi provissioning\nGive Credentials of your access point using smartphone app");
        break;

      case ARDUINO_EVENT_PROV_CRED_RECV: {
        if (debug) Serial.println("\nReceived Wi-Fi credentials");
        if (debug) Serial.print("\tSSID : ");
        if (debug) Serial.println((const char *) sys_event->event_info.prov_cred_recv.ssid);
        if (debug) Serial.print("\tPassword : ");
        if (debug) Serial.println((char const *) sys_event->event_info.prov_cred_recv.password);
        if (debug) Serial.println("");
        break;
      }

      case ARDUINO_EVENT_WIFI_STA_GOT_IP:
        if (debug) Serial.print("Wi-Fi Connected.\n\tIP address : ");
        if (debug) Serial.println(IPAddress(sys_event->event_info.got_ip.ip_info.ip.addr));
        digitalWrite(led_wifi, HIGH);
        break;

      case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
        if (debug) Serial.println("Wi-Fi Disconnected");
        digitalWrite(led_wifi, LOW);

      case ARDUINO_EVENT_PROV_CRED_FAIL: {
        if (debug) Serial.println("Provisioning failed!");
        if(sys_event->event_info.prov_fail_reason == WIFI_PROV_STA_AUTH_ERROR) {
          if (debug) Serial.println("Wi-Fi password incorrect");
          if (debug) Serial.println("Resetting WiFi provissioning");
          wifi_prov_mgr_reset_provisioning();
          ESP.restart();
        }
        else {
          if (debug) Serial.println("Wi-Fi AP not found.");
          if (debug) Serial.println("kindly check your Access Point");
        }
        break;
      }

      case ARDUINO_EVENT_PROV_CRED_SUCCESS:
        if (debug) Serial.println("Provisioning Successful");
        break;

      default: break;
    }
}


//Get switch State from Blynk
BLYNK_WRITE(vpin_relay) {
  btn_state = param.asInt();
  pref.putBool("btn", btn_state);
}


//showing the parameter values from the server onto the LCD
void showparam(){
if (server_flag) {

  BLYNK_WRITE(vpin_current) {
    String i = param.asStr();
    lcd2.setCursor(0,0);
    lcd2.print(i);
    lcd2.setCursor(4,0);
    lcd2.print("mA")
    }

  BLYNK_WRITE(vpin_voltage){
    String j = param.asStr();
    lcd2.setCursor(6,0);
    lcd2.print(j);
    lcd2.print("V"); 
  }

  BLYNK_WRITE(vpin_POWER) {
    String K = param.asStr();
    lcd2.setCursor(0,0);
    lcd2.print(K);
    lcd2.setCursor(4,0);
    lcd2.print("mA")
    }

  }
}


//Check Blynk Status
void checkBlynkStatus() {
  bool isconnected = Blynk.connected();
  if (isconnected != pref.getBool("server")){
    if (isconnected == false) {
      server_flag = 0;
      digitalWrite(led_server, LOW);
      if (debug) Serial.println("Blynk Not Connected");
    }
    if (isconnected == true) {
      server_flag = 1;
      digitalWrite(led_server, HIGH);
      if (debug) Serial.println("Blynk Connected");
    }

    pref.putBool("server", isconnected);

  }
}


//Getting switch state and syncing with blynk
void SwitchState() {
  btn_state = pref.getBool("btn");
  if (server_flag == 1) {
    Blynk.virtualWrite(vpin_relay, btn_state);
  }
}


//Switching the transmitter via reciever
void Switch_tx() {
  if (digitalRead(btn) == LOW && btn_state == LOW) {
      btn_state = HIGH;
      pref.putBool("btn", btn_state);

      if (server_flag == 1) {
        Blynk.virtualWrite(vpin_relay, HIGH);
      }
      //lcd on device
      if (debug) Serial.println("Switch on");
    }

  if (digitalRead(btn) == HIGH && btn_state == HIGH) {
      btn_state = LOW;
      pref.putBool("btn", btn_state);

      if (server_flag == 1) {
        Blynk.virtualWrite(vpin_relay, btn_state);
      }

      //lcd off device
      if (debug) Serial.println("Switch off");
    }

}


//Main Setup
void setup() {

  Serial.begin(115200);

  lcd1.begin();
  lcd2.begin();

  lcd2.setCursor(0,0);
  lcd2.print("   BOOTING UP   ");
  lcd2.setCursor(0,1);
  lcd2.print(". . . . . . . .");

  lcd1.setCursor(0,0);
  lcd1.print("    OORJA");
  lcd1.setCursor(0,1);
  lcd1.print("Power Monitoring");

  pinMode(btn, INPUT_PULLUP);

  lcd2.backlight();
  lcd1.backlight();


  pinMode(debug_led, OUTPUT);
  pref.begin("State", false);

  btn_state_prev = pref.getBool("btn");
  debug = pref.getBool("debug");

  if (debug) Serial.println("Initialising Wi-Fi");
  WiFi.onEvent(SysProvEvent);
  if (debug) Serial.println("Searching for previous networks");

  WiFiProv.beginProvision(WIFI_PROV_SCHEME_SOFTAP, WIFI_PROV_SCHEME_HANDLER_NONE, WIFI_PROV_SECURITY_1, pop, service_name, service_key);
  log_d("wifi qr");
  if (debug) WiFiProv.printQR(service_name, pop, "softap");

  Blynk.config(auth);
  pref.putBool("server", false);

  timer.setInterval(1000L, checkBlynkStatus);
  timer.setInterval(1000L, checkparam);

  if (debug) Serial.println("Successfully Booted");
  if (debug) Serial.println("Wi-Fi Disconnected");
  if (debug) Serial.println("Blynk Not Connected");
  delay(1000);

}


//Main Loop
void loop(){

  Blynk.run();
  timer.run();
  Switch_tx();


  //Debug Mode
  if (!digitalRead(debug_btn)) {
    if(!debug) {
      digitalWrite(debug_led, HIGH);
      delay(50);
      digitalWrite(debug_led, LOW);
      delay(50);
      digitalWrite(debug_led, HIGH);
      delay(50);
      digitalWrite(debug_led, LOW);
      delay(2000);
      if (!digitalRead(debug_btn)) {
        digitalWrite(debug_led, HIGH);
        pref.putBool("debug", true);
        ESP.restart();
        Blynk.disconnect();
      }
    }

    if(debug) {
      digitalWrite(debug_led, LOW);
      delay(50);
      digitalWrite(debug_led, HIGH);
      delay(50);
      digitalWrite(debug_led, LOW);
      delay(50);
      digitalWrite(debug_led, HIGH);
      delay(2000);
      if (!digitalRead(debug_btn)) {
        digitalWrite(debug_led, LOW);
        pref.putBool("debug", false);
        ESP.restart();
        Blynk.disconnect();
      }
    }
  }

}