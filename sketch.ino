#include <WiFi.h>
#include <ESP32Servo.h>
#include <PubSubClient.h>

const int pinDisparoUltrasonico = 5;    // Sensor Ultrasónico - Trig
const int pinRecepcionUltrasonico = 18; // Sensor Ultrasónico - Echo
const int pinSensorMovimiento = 4;      // Sensor de Movimiento PIR - OUT
const int pinBuzzerAlerta = 19;         // Buzzer - GPIO 19
const int pinControlServo = 23;         // Servomotor - PWM

Servo motorCompuerta;

const char* redWifi = "Wokwi-GUEST";
const char* claveWifi = "";

const char* mqttServer = "broker.emqx.io";
const int mqttPort = 1883;
const char* mqttTopic = "dispensador/mascotas/telemetria/mi-dispensador-123";

WiFiClient espClient;
PubSubClient client(espClient);

const int distanciaMinimaComida = 2;
const int distanciaMaximaComida = 60;

void conectarMQTT() {
  while (!client.connected()) {
    String clientId = "ESP32-Mascota-" + String(random(1000, 9999));
    if (client.connect(clientId.c_str())) {
      Serial.println("Conectado a MQTT!");
    } else {
      delay(1000);
    }
  }
}

void publicarEstado(int dist, int pir, const char* compuerta, const char* evento) {
  if (!client.connected()) {
    conectarMQTT();
  }
  client.loop();
  String payload = "{\"distancia\":" + String(dist) +
                   ",\"pir\":" + String(pir) +
                   ",\"compuerta\":\"" + String(compuerta) + "\"" +
                   ",\"evento\":\"" + String(evento) + "\"}";
  client.publish(mqttTopic, payload.c_str());
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(pinDisparoUltrasonico, OUTPUT);
  pinMode(pinRecepcionUltrasonico, INPUT);
  pinMode(pinSensorMovimiento, INPUT);
  pinMode(pinBuzzerAlerta, OUTPUT);

  motorCompuerta.attach(pinControlServo);
  motorCompuerta.write(0);

  Serial.print("Conectando a Wi-Fi: ");
  Serial.println(redWifi);
  WiFi.begin(redWifi, claveWifi);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n¡Conectado a la red Wi-Fi!");

  client.setServer(mqttServer, mqttPort);
  conectarMQTT();
}

void loop() {
  if (!client.connected()) {
    conectarMQTT();
  }
  client.loop();

  int mascotaDetectada = digitalRead(pinSensorMovimiento);

  digitalWrite(pinDisparoUltrasonico, LOW);
  delayMicroseconds(2);
  digitalWrite(pinDisparoUltrasonico, HIGH);
  delayMicroseconds(10);
  digitalWrite(pinDisparoUltrasonico, LOW);

  long duracionPulso = pulseIn(pinRecepcionUltrasonico, HIGH, 30000);
  int distanciaTolvaCm = (duracionPulso > 0) ? (duracionPulso * 0.034 / 2) : 999;

  Serial.print("Distancia tolva: ");
  Serial.print(distanciaTolvaCm);
  Serial.print(" cm | PIR: ");
  Serial.println(mascotaDetectada);

  const char* estadoCompuerta = "CERRADA (0°)";

  if (mascotaDetectada == HIGH) {
    if (distanciaTolvaCm >= distanciaMinimaComida && distanciaTolvaCm < distanciaMaximaComida) {
      Serial.println("¡Mascota detectada y tolva con comida! Abriendo compuerta...");
      digitalWrite(pinBuzzerAlerta, HIGH);
      delay(400);
      digitalWrite(pinBuzzerAlerta, LOW);

      motorCompuerta.write(90);
      publicarEstado(distanciaTolvaCm, mascotaDetectada, "ABIERTA (90°)", "COMPUERTA ABIERTA (90°)");
      delay(3000);

      motorCompuerta.write(0);
      Serial.println("Compuerta cerrada.");
      publicarEstado(distanciaTolvaCm, mascotaDetectada, "CERRADA (0°)", "COMPUERTA CERRADA (0°)");
      delay(1500);
    } else {
      Serial.println("⚠️ ¡AVISO! Mascota detectada, pero NO HAY COMIDA en la tolva.");
      publicarEstado(distanciaTolvaCm, mascotaDetectada, "BLOQUEADA (0°)", "ALERTA: SIN COMIDA (COMPUERTA BLOQUEADA)");

      for (int i = 0; i < 3; i++) {
        digitalWrite(pinBuzzerAlerta, HIGH);
        delay(200);
        digitalWrite(pinBuzzerAlerta, LOW);
        delay(200);
      }
      delay(1000);
    }
  } else {
    publicarEstado(distanciaTolvaCm, mascotaDetectada, estadoCompuerta, "MONITOREO");
  }

  delay(800);
}