#include <WiFi.h>
#include <PubSubClient.h>

const int GAS_PIN = 32; // Pin analógico para el sensor de gas
const int BUZZER_PIN = 12;
const int LED_PIN = 14;
const char* ssid = "9JS";
const char* password = "lol123456";
const char* mqtt_server = "172.191.237.33"; // TODO: Cambiar IP Servidor 

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
float gasValue;
float temp = 0;

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("Connected");
      // client.publish("uts-iot/mqtt-jaider/gas-detection", "Welcome ESP32 to Gas Detection");
      client.subscribe("uts-iot/mqtt-jaider/gas-detection");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  Serial.println("MQ2 warming up!");
  delay(20000); 
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  unsigned long now = millis();

  if (now - lastMsg > 3000) {
    lastMsg = now;
    gasValue = analogRead(GAS_PIN);

    if (gasValue < 3500) {
      client.publish("uts-iot/mqtt-jaider/gas-detection", "0");
      client.publish("uts-iot/mqtt-jaider/state-gas-detection", "No detected");
      Serial.println("\nGas: No detected");
      Serial.println(gasValue);
      digitalWrite(LED_PIN, LOW);
    } else {
      client.publish("uts-iot/mqtt-jaider/gas-detection", "1");
      client.publish("uts-iot/mqtt-jaider/state-gas-detection", "Detected");
      tone(BUZZER_PIN, 100);
      delay(500);
      tone(BUZZER_PIN, 200);
      delay(500);
      tone(BUZZER_PIN, 0);
      digitalWrite(LED_PIN, HIGH);
      Serial.println("\nGas: Detected");
      Serial.println(gasValue);
    }
  }
}
