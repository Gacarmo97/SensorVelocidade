// KY-040 ...  ESP32
// CLK    ...  PIN 4
// DT     ...  PIN 2
// SW     ...  PIN 5
// +      ...  3.3V
// GND    ...  GND

#include <Arduino.h>
#include "EspMQTTClient.h"

EspMQTTClient client(
  "Wokwi-GUEST",         // SSID WiFi
  "",                    // Password WiFi
  "test.mosquitto.org",  // MQTT Broker
  "mqtt-wokwi",          // Client
  1883                  // MQTT port
);

long int rotValue=0, swValue=0;
uint8_t state=0;

#define ROTARY_PINA 2
#define ROTARY_PINB 4
#define ROTARY_PINSW 5
#define LED_VERMELHO 21
#define LED_AMARELO 19
#define LED_VERDE 18

portMUX_TYPE gpioMux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR isrAB() {
   uint8_t s = state & 3;

  portENTER_CRITICAL_ISR(&gpioMux);
    if (digitalRead(ROTARY_PINA)) s |= 4;
    if (digitalRead(ROTARY_PINB)) s |= 8;
    switch (s) {
      case 0: case 5: case 10: case 15:
        break;
      case 1: case 7: case 8: case 14:
        rotValue++; break;
      case 2: case 4: case 11: case 13:
        rotValue--; break;
      case 3: case 12:
        rotValue += 2; break;
      default:
        rotValue -= 2; break;
    }
    state = (s >> 2);
   portEXIT_CRITICAL_ISR(&gpioMux);
}

void IRAM_ATTR isrSWAll() {

 portENTER_CRITICAL_ISR(&gpioMux);
 swValue++;
 portEXIT_CRITICAL_ISR(&gpioMux);

}

void semaforo (int velocidade){
  if (velocidade < 50) {
    digitalWrite(LED_VERMELHO, LOW);
    digitalWrite(LED_AMARELO, LOW);
    digitalWrite(LED_VERDE, HIGH);
  }
  else if (velocidade >= 50 & velocidade < 100){
    digitalWrite(LED_VERDE, LOW);
    digitalWrite(LED_VERMELHO, LOW);
    digitalWrite(LED_AMARELO, HIGH);
  }
  else if (velocidade >= 100){
    digitalWrite(LED_VERDE, LOW);
    digitalWrite(LED_VERMELHO, HIGH);
    digitalWrite(LED_AMARELO, LOW);
  }
}

void lerEnviarDados() {
  Serial.println(rotValue);
  Serial.println(swValue);
  Serial.println("---");
  client.publish("topicowokwi/VelocidadeMackenzie", String(rotValue, 2));
}

void onConnectionEstablished()
{
  // Subscribe no "topicowokwi/msgRecebida/#" e mostra a mensagem recebida na Serial
    client.subscribe("topicowokwi/msgRecebidaMackenzie/#", [](const String & topic, const String & payload) {
    Serial.println("Mensagem recebida no topic: " + topic + ", payload: " + payload);
  });
  lerEnviarDados();
}

void setup(){
    pinMode(ROTARY_PINA, INPUT_PULLUP);
    pinMode(ROTARY_PINB, INPUT_PULLUP);
    pinMode(ROTARY_PINSW, INPUT_PULLUP);
    pinMode(LED_VERMELHO, OUTPUT);
    pinMode(LED_AMARELO, OUTPUT);
    pinMode(LED_VERDE, OUTPUT);

    attachInterrupt(ROTARY_PINA, isrAB, CHANGE);
    attachInterrupt(ROTARY_PINB, isrAB, CHANGE);
    attachInterrupt(ROTARY_PINSW, isrSWAll, CHANGE);
    Serial.begin(115200);

    client.enableDebuggingMessages(); // Enable debugging messages sent to serial output
    client.enableHTTPWebUpdater(); // Enable the web updater. User and password default to values of MQTTUsername and MQTTPassword. These can be overridded with enableHTTPWebUpdater("user", "password").
    client.enableOTA(); // Enable OTA (Over The Air) updates. Password defaults to MQTTPassword. Port is the default OTA port. Can be overridden with enableOTA("password", port).
    client.enableLastWillMessage("TestClient/lastwill", "Vou ficar offline");

}

void loop()
{
 
client.loop();
semaforo(rotValue);

}