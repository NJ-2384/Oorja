//IO Pins
#define relay 4
#define buzzer 5
#define led_power 16
#define led_wifi 17
#define led_server 18
#define led_switch 19
#define reset_btn 25
#define switch_btn 26
#define debug_btn 0
#define debug_led 2

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
#include <PZEM004Tv30.h>


//Wi-Fi Provisioning Variables
const char * pop = "Esp_TX32";
const char * service_name = "PROV_espTX";
const char * service_key = NULL;
bool reset_provisioned = false;

//Blynk Variables
char auth[] = BLYNK_AUTH_TOKEN;
int server_flag;

//IO Variables
bool btn_state = LOW;
bool relay_state = LOW;
bool debug;

//PZEM Variables
float current = 0;
float voltage = 0;
float power = 0;
float pf = 0;
float frequency = 0;


//Instances
PZEM004Tv30 pzem(Serial2, 3, 1);
BlynkTimer timer;
Preferences pref;


//Get Relay State from Blynk
BLYNK_WRITE(vpin_relay) {
  relay_state = param.asInt();
  digitalWrite(relay, !relay_state);
  digitalWrite(led_switch, relay_state);
  pref.putBool("Relay", relay_state);
}


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


//Sync Virtual Values with Blynk
BLYNK_CONNECTED() {
  Blynk.virtualWrite(vpin_relay, relay_state);
  Blynk.syncVirtual(vpin_info);
  Blynk.syncVirtual(vpin_current);
  Blynk.syncVirtual(vpin_voltage);
  Blynk.syncVirtual(vpin_power);
}


//Reading PZEM Sensors
void emon() {
  voltage = pzem.voltage();
  if(isnan(voltage)){
    voltage = 0.0;
    if (debug) Serial.println("Error calculating Voltage");
    Blynk.logEvent("sensor_malfunction");
  }

  current = pzem.current();
  if(isnan(current)){
    current = 0.000;
    if (debug) Serial.println("Error calculating Current");
    Blynk.logEvent("sensor_malfunction");
  }

  power = pzem.power();
  if(isnan(power)){
    power = 0.0;
    if (debug) Serial.println("Error calculating Power");
    Blynk.logEvent("sensor_malfunction");
  }

  pf = pzem.pf();
  if(isnan(pf)){
    pf = 0.0;
    if (debug) Serial.println("Error calculating Power Factor");
    Blynk.logEvent("sensor_malfunction");
  }

  frequency = pzem.frequency();
  if(isnan(frequency)){
    frequency = 0.0;
    if (debug) Serial.println("Error calculating Frequency");
    Blynk.logEvent("sensor_malfunction");
  }
}


//Sending sensor value to Blynk
void transmit() {
  if (server_flag == 1) {
    emon();

    Blynk.virtualWrite(vpin_current, current);
    Blynk.virtualWrite(vpin_voltage, voltage);
    Blynk.virtualWrite(vpin_power, power);

    String pfs = String(pf,2);
    String frs = String(frequency,1);
    String info = String("pf = " + pfs + " Frequency = " + frs + " Hz");

    Blynk.virtualWrite(vpin_info, info);
  }

}


//Manually switching the circuit
void switch_relay() {
  if (digitalRead(switch_btn) == LOW && btn_state == LOW) {
      digitalWrite(relay, LOW);
      relay_state = HIGH;
      btn_state = HIGH;
      pref.putBool("Relay", relay_state);

      if (server_flag == 1) {
        Blynk.virtualWrite(vpin_relay, relay_state);
      }
      
      digitalWrite(led_switch, HIGH);
      if (debug) Serial.println("Switch on");
    }

  if (digitalRead(switch_btn) == HIGH && btn_state == HIGH) {
      digitalWrite(relay, HIGH);
      relay_state = LOW;
      btn_state = LOW;
      pref.putBool("Relay", relay_state);

      if (server_flag == 1) {
        Blynk.virtualWrite(vpin_relay, relay_state);
      }

      digitalWrite(led_switch, LOW);
      if (debug) Serial.println("Switch off");
    }

}


//Getting relay states and syncing with blynk
void RelayState() {
  relay_state = pref.getBool("Relay");
  digitalWrite(relay, !relay_state);
  digitalWrite(led_switch, relay_state);

  if (server_flag == 1) {
    Blynk.virtualWrite(vpin_relay, relay_state);
  }
}


//Main Setup
void setup() {

  Serial.begin(115200);

  pinMode(led_power, OUTPUT);
  digitalWrite(led_power, HIGH);

  pinMode(led_wifi, OUTPUT);
  digitalWrite(led_wifi, LOW);

  pinMode(led_server, OUTPUT);
  digitalWrite(led_server, LOW);

  pinMode(led_switch, OUTPUT);
  digitalWrite(led_switch, LOW);

  pinMode(debug_led, OUTPUT);

  pinMode(buzzer, OUTPUT);
  pinMode(relay, INPUT);
  pinMode(reset_btn, INPUT_PULLUP);

  pref.begin("State", false);
  debug = pref.getBool("debug");
  if (debug) digitalWrite(led_power, HIGH);

  if (debug) digitalWrite(debug_led, HIGH);
  if (!debug) digitalWrite(debug_led, LOW);

  if (debug) Serial.println("Booting");

  if (debug) Serial.println("Initialising Wi-Fi");
  WiFi.onEvent(SysProvEvent);
  if (debug) Serial.println("Searching for previous networks");

  digitalWrite(led_power, HIGH);

  pinMode(switch_btn, INPUT_PULLUP);
  digitalWrite(relay, !relay_state);

  WiFiProv.beginProvision(WIFI_PROV_SCHEME_SOFTAP, WIFI_PROV_SCHEME_HANDLER_NONE, WIFI_PROV_SECURITY_1, pop, service_name, service_key);
  log_d("wifi qr");
  if (debug) WiFiProv.printQR(service_name, pop, "softap");

  digitalWrite(led_power, LOW);

  Blynk.config(auth);
  pref.putBool("server", false);

  timer.setInterval(1000L, checkBlynkStatus);
  timer.setInterval(1000L, transmit);
  RelayState();
  
  if (debug) Serial.println("Successfully Booted");
  if (debug) Serial.println("Wi-Fi Disconnected");
  if (debug) Serial.println("Blynk Not Connected");
  digitalWrite(led_power, HIGH);

}


//Main Loop
void loop() {

  Blynk.run();
  timer.run();
  switch_relay();


  //Reset Wi-Fi Provisioning
  if (!digitalRead(reset_btn)) {
    digitalWrite(led_wifi, LOW);
    delay(100);
    digitalWrite(led_wifi, HIGH);
    delay(2000);
    if (!digitalRead(reset_btn)) {
      if (debug) Serial.println("Reseting Wi-Fi Provisioning");

      digitalWrite(led_wifi, LOW);
      delay(100);
      digitalWrite(led_wifi, HIGH);
      delay(100);
      digitalWrite(led_wifi, LOW);
      delay(100);
      digitalWrite(led_wifi, HIGH);
      delay(100);
      digitalWrite(led_wifi, LOW);
      delay(100);
      digitalWrite(led_wifi, HIGH);

      wifi_prov_mgr_reset_provisioning();

      if (debug) Serial.println("Rebooting the ESP");
      ESP.restart();
      Blynk.disconnect();
    }
  }


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


  //Over Current protection
  if (current > 9.9) {
    relay_state = LOW;
    digitalWrite(relay, !relay_state);
    pref.putBool("relay", relay_state);
    digitalWrite(buzzer, HIGH);
    delay(5000);
    digitalWrite(buzzer, LOW);
  }


  // Over Voltage protection
  if (voltage > 225) {
    relay_state = LOW;
    digitalWrite(relay, !relay_state);
    pref.putBool("relay", relay_state);
    digitalWrite(buzzer, HIGH);
    delay(5000);
    digitalWrite(buzzer, LOW);
  }

}