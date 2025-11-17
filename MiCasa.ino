#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ESP32Servo.h>

// ...existing code...

// --- HARDWARE CONFIGURATION ---
#define PIR_PIN 23        // Pin for the PIR motion sensor
#define LED_PIN 22        // Pin for the LED
#define SERVO_PIN 21      // Pin for the servo motor
#define SERVO_OPEN_POS 90   // Position in degrees for "door open"
#define SERVO_CLOSED_POS 0 // Position in degrees for "door closed"

// --- NETWORK AND AWS CONFIGURATION ---
const char* WIFI_SSID = "Flia.Torrez.Llanos"; // <--- FILL THIS
const char* WIFI_PASS = "Entel@2022"; // <--- FILL THIS

const char* MQTT_BROKER = "a30eisqpnafvzg-ats.iot.us-east-1.amazonaws.com"; // <--- FILL WITH YOUR ENDPOINT
const char* THING_NAME = "MiCasa"; // <--- MAKE SURE THIS MATCHES YOUR THING AND LAMBDA

// --- AWS CERTIFICATES ---
// Amazon root certificate
const char AMAZON_ROOT_CA1[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF
ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6
b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL
MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv
b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj
ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM
9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw
IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6
VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L
93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm
jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC
AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA
A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI
U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs
N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv
o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU
5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy
rqXRfboQnoZsG4q5WTP468SQvvG5
-----END CERTIFICATE-----
)EOF";

// Device (Thing) certificate
const char CERTIFICATE[] PROGMEM = R"KEY(
-----BEGIN CERTIFICATE-----
MIIDWTCCAkGgAwIBAgIUcka0Kb1nuTZpIctqVn42GcrNIZwwDQYJKoZIhvcNAQEL
BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g
SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTI1MTExNDE5MjIy
MloXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0
ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAJ2MS9HYkpiAQUJfWOrQ
gnfY0Rl0Lav3BOBfDq3+CQNanDtUbeEnY58wfoRq6F4ImpNbZK/nlSE3FyzbnQzl
Dft3gCKSQn5BVKstKK/c+zPyt6D+zDT4Vd6FbOwFPk+VLdnQRJo+SdlnQf6cEZ9r
rOcZbVaO7nuYwMvRsY/wjYee9sGvUYbGxnTnt9UZazYhfQ0uHSTxSmXWwaZcpk8t
RxVqj6tBLVu3rxyOaGOJ3iV6ajb7u9VFE0uY5s77F7MDxb8XI1NvKg7dgxe0LA5p
3wo5igkIYfL5KpQIGerVMEQhr+s8VzsUvNqI+KRIOSJ1h/CDWtthN6xeiza0xMdo
xmMCAwEAAaNgMF4wHwYDVR0jBBgwFoAUnMjszMM/GKhEl00D8AU9DedWpdMwHQYD
VR0OBBYEFBMWVJqpBMwAnWnxbnfSzzZAje+EMAwGA1UdEwEB/wQCMAAwDgYDVR0P
AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQCoz1EJAkywtdHwGcP9mjYIQchO
zvc/MaEGu2oMMPcJgbjh8zYu6ebHcCRcQ4h3t7QGC/k9H/0R0NcZgpzrA7hG4DOz
ZLTjhuXloxQXpadjgeRzEtLbsQjz0FcFiS5uORFPSCNsnIFMo6eaIfemeqsQsuc1
uEs+oBzi0vwmVYmJEWK5XdNNS8nFpZO1/K//vW9Su1k5wj7i51hm57OFz/OSZJWl
bh1tgz/4Jy4lfE046t0r3r8H3EpFWG18sja+mKdL33e0Jwk+AB/i37UiES3NvtS8
d2z6HxJQCXUb+yO+6/6vc3MSJwsQEyXlQAbmdb3i+TCbM/H6aHkqZ0ZfNM1T
-----END CERTIFICATE-----
)KEY";

// Device (Thing) private key
const char PRIVATE_KEY[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----
MIIEpAIBAAKCAQEAnYxL0diSmIBBQl9Y6tCCd9jRGXQtq/cE4F8Orf4JA1qcO1Rt
4SdjnzB+hGroXgiak1tkr+eVITcXLNudDOUN+3eAIpJCfkFUqy0or9z7M/K3oP7M
NPhV3oVs7AU+T5Ut2dBEmj5J2WdB/pwRn2us5xltVo7ue5jAy9Gxj/CNh572wa9R
hsbGdOe31RlrNiF9DS4dJPFKZdbBplymTy1HFWqPq0EtW7evHI5oY4neJXpqNvu7
1UUTS5jmzvsXswPFvxcjU28qDt2DF7QsDmnfCjmKCQhh8vkqlAgZ6tUwRCGv6zxX
OxS82oj4pEg5InWH8INa22E3rF6LNrTEx2jGYwIDAQABAoIBABil01LQLBcZsZv9
qL6Lwm2XiRcPWvnVWnjzxisoXCafLWQG0G3yKJ7GCnUS+KmFdRFqsfTnKSiaItEM
xqQ2zZoy1uQRt79i3ykslnn0+4PkDCBu3GnJFD4M576BD3+J2jOCZskux4Twp516
szMauGErVDS/hcXmXvtEIiBfxPSbhiWYuGAIhMJdzETnWYYNzLleeWT6E79nC28o
eCSIvccZ/jaD7kN9MN+Wo65lSeYByyR2kzg/J+omnwGlAW+3DjptYQXC1/d/1tNT
2dzgjG+TbC2lkx2dxNwqhpz/PDId2yD89xatNMbmwWOtUt9GwmiFVWBdrflxAU/N
UnUJloECgYEAz6SHeA0YL23sVNjGGWPhuX9sVCNx8NR0fe61yxl6DMqSDvNuL5RN
cA2o6V0BMzKLilUsoq4uV0RPqADjLwQvM0beNl5Dk6fF1KhoWwZjoxOO91QKRU/2
NDkn2qEOL3WyzUFj+fwBTz5L3a2R2pOsIhir0wAXITwW9pvTxGHX9EECgYEAwj0r
GJnxlcgfczKQ7kM4+mFCLZeFLKNQ9y86NjG1RG70olzWCwBuaC+98I42XWo9RXJb
hyR7k9JjzsQBgqT7BlXgM7bgsHxrl3wWh4gNMUyJK++MqW2h2US+kguJAIXfvGp5
KH9P3XcyiVZ5FVxVwKVWZa64Nt6Qqzso2pX0AaMCgYEAnklW975LrIRIP1nMkbhh
b03in7Uxe5wJfaKGNOAj+TJG61zIpz6PVZckJ1k9u/CRDQ/m6dCMBPxPGMwsYz/2
V0lzxdBQBXREy9rhSlpxg/Q57PLErZfKIH9dkT1rGeTIO83YT826ldTwcnmJES23
b3qNKVkNSMmM28toQHEx9cECgYA2Oo+iobAiCipFpjNyYdAxjx/DzM7AhKQhLs1I
vh4GihfAkLyte6RXcCTOWVXyRKvwjJnjJBtgTNfSrURirh7rSOR8a61VeJctRtaf
ZiuQvPZ7DrktwE70cs3lxlSskbPCTdOfq9OnWBUC5OKPRSUn0MFLSDlkEEVBmRWx
2TWzkwKBgQCOkAkAcpW2gOqK1Pt3Wq+/xqM+7dm35TR2xSDfGtm56eMj4XYSnqtj
2lroiGxUth/1ghsJZNnVJTBTXfOXdM1vZegJMJe4koPhX8neOsQb0TkB4xNqkD0M
nJpEIzLfc/oMZjHxg2oJAL7Mt9g2rbJKuxCWZWHiHBH8sDCwJRms4w==
-----END RSA PRIVATE KEY-----
)KEY";

// --- MQTT TOPICS FOR THE SHADOW ---
const char* UPDATE_TOPIC = "$aws/things/MiCasa/shadow/update";
const char* UPDATE_DELTA_TOPIC = "$aws/things/MiCasa/shadow/update/delta";

// --- GLOBAL CLIENTS AND OBJECTS ---
WiFiClientSecure wiFiClient;
PubSubClient mqttClient(wiFiClient);
Servo myServo;

// --- STATE VARIABLES ---
String ledState = "OFF";
String doorState = "CLOSED";
String motionState = "NOT_DETECTED";
String lastMotionState = "NOT_DETECTED"; // To detect changes

// --- FUNCTION DECLARATIONS ---
void publishShadowUpdate();

// --- MQTT MESSAGE CALLBACK FUNCTION ---
// Executes every time a message from the 'delta' topic arrives
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println("-----------------------");
  Serial.print("Mensaje recibido del tema: ");
  Serial.println(topic);
  
  StaticJsonDocument<512> doc;
  deserializeJson(doc, payload, length);
  
  JsonObject state = doc["state"];
  bool stateChanged = false; // Flag to know if we must report a change

  // --- LED logic ---
  if (state.containsKey("ledState")) {
    String desiredLedState = state["ledState"].as<String>();
    Serial.print("Estado deseado para el LED: ");
    Serial.println(desiredLedState);
    if (desiredLedState == "ON" && ledState != "ON") {
      digitalWrite(LED_PIN, HIGH);
      ledState = "ON";
      stateChanged = true;
    } else if (desiredLedState == "OFF" && ledState != "OFF") {
      digitalWrite(LED_PIN, LOW);
      ledState = "OFF";
      stateChanged = true;
    }
  }
  
  // --- Door (Servo) logic ---
  if (state.containsKey("doorState")) {
    String desiredDoorState = state["doorState"].as<String>();
    Serial.print("Estado deseado para la Puerta: ");
    Serial.println(desiredDoorState);
    if (desiredDoorState == "OPEN" && doorState != "OPEN") {
      myServo.write(SERVO_OPEN_POS);
      doorState = "OPEN";
      stateChanged = true;
    } else if (desiredDoorState == "CLOSED" && doorState != "CLOSED") {
      myServo.write(SERVO_CLOSED_POS);
      doorState = "CLOSED";
      stateChanged = true;
    }
  }

  // If any state changed due to an Alexa command, publish the new reported state
  if (stateChanged) {
    Serial.println("El estado ha cambiado por una orden, publicando actualización...");
    publishShadowUpdate();
  }
}

// --- FUNCTION TO PUBLISH CURRENT STATE TO THE SHADOW ---
void publishShadowUpdate() {
  StaticJsonDocument<512> jsonDoc;
  JsonObject state = jsonDoc.createNestedObject("state");
  JsonObject reported = state.createNestedObject("reported");

  reported["ledState"] = ledState;
  reported["doorState"] = doorState;
  reported["motion"] = motionState;

  char jsonBuffer[512];
  serializeJson(jsonDoc, jsonBuffer);

  Serial.print("Publicando en el Shadow: ");
  Serial.println(jsonBuffer);
  
  mqttClient.publish(UPDATE_TOPIC, jsonBuffer);
}

// --- FUNCTION TO CHECK THE MOTION SENSOR ---
void checkMotionSensor() {
  if (digitalRead(PIR_PIN) == HIGH) {
    motionState = "DETECTED";
  } else {
    motionState = "NOT_DETECTED";
  }

  // If the sensor state changed, publish it
  if (motionState != lastMotionState) {
    Serial.print("¡Cambio en el sensor de movimiento! Nuevo estado: ");
    Serial.println(motionState);
    publishShadowUpdate();
    lastMotionState = motionState; // Update the last known state
  }
}

void setupWiFi() {
  Serial.print("Conectando a Wi-Fi: ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n¡Conectado a la red Wi-Fi!");
  Serial.print("Dirección IP: ");
  Serial.println(WiFi.localIP());
}

void reconnectMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Intentando conexión MQTT...");
    if (mqttClient.connect(THING_NAME)) {
      Serial.println("¡Conectado a AWS IoT!");
      // We subscribe to the 'delta' topic to receive Alexa's commands
      mqttClient.subscribe(UPDATE_DELTA_TOPIC);
      Serial.print("Suscrito a: ");
      Serial.println(UPDATE_DELTA_TOPIC);
      // We publish our initial state upon connecting
      publishShadowUpdate();
    } else {
      Serial.print("falló, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" | Intentando de nuevo en 5 segundos");
      delay(5000);
    }
  }
}

// --- MAIN LOOP FUNCTION ---
void setup() {
  Serial.begin(115200);
  delay(1000);

  // Pin configuration
  pinMode(LED_PIN, OUTPUT);
  pinMode(PIR_PIN, INPUT);
  myServo.attach(SERVO_PIN);
  
  // Initial state of actuators
  digitalWrite(LED_PIN, LOW);
  myServo.write(SERVO_CLOSED_POS);

  // Network and AWS connection
  setupWiFi();
  wiFiClient.setCACert(AMAZON_ROOT_CA1);
  wiFiClient.setCertificate(CERTIFICATE);
  wiFiClient.setPrivateKey(PRIVATE_KEY);

  mqttClient.setServer(MQTT_BROKER, 8883);
  mqttClient.setCallback(callback); // Assign the function that will handle messages
}

// --- MAIN LOOP FUNCTION ---
void loop() {
  if (!mqttClient.connected()) {
    reconnectMQTT();
  }
  mqttClient.loop(); // Maintains the connection and processes incoming messages

  checkMotionSensor(); // We check the motion sensor continuously
  delay(200); // Small pause to avoid overload
}