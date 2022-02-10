#include "credentials.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

const char* ssid = CRED_SSID;
const char* password = CRED_PASSWORD;
const char* mqtt_server = CRED_MQTT_SERVER;
const int mqtt_port = CRED_MQTT_PORT;
const char* mqtt_user = CRED_MQTT_USER;
const char* mqtt_pwd = CRED_MQTT_PASSWD;
const char* clientId = CRED_CLIENT_ID;
const char* pub_topic = CRED_PUB_TOPIC;

WiFiClient espClient;
PubSubClient client(espClient);

#define DHTPIN D1
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

float temp = 0.0;
float hum = 0.0;
const int sleepMinutes = 1;

void connectWifi(){
  Serial.println();
  Serial.print("Connecting to: ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  int count = 0 ;
  while (WiFi.status() != WL_CONNECTED && count <= 15) {
    delay(1000);
    Serial.print(".");
    count++;
  }
  
  if(WiFi.status() != WL_CONNECTED){
    handleError("Failed to connect to Wifi.");
  } else{
  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();  
  successBlink();
  }
}

void readDhtSensor (){
  float newTemp = dht.readTemperature();
  float newHum = dht.readHumidity();

  if(isnan(newTemp)||isnan(newHum)){
    handleError("Failed to read from DHT sensor.");
  }else{
    temp = newTemp;
    hum = newHum;
    successBlink();
  }
}

void publishData(){
    Serial.println("Connecting to MQTT server...");
    if (client.connect(clientId, mqtt_user, mqtt_pwd)) {
      Serial.println("MQTT connected");
      successBlink();
      char payload[40];
      sprintf(payload, "{\"temperature\": %.1f, \"humidity\": %.1f}", temp, hum);
      boolean published = client.publish(pub_topic, payload);
      delay(500);
      if(published){
      Serial.println("Sensor data was published:");
      Serial.println(payload); 
      successBlink();
      } else{
        handleError("Failed to publish sensor data");
      }
    } else {
      handleError("Failed to connect to MQTT server.");
    }
}

void successBlink(){
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);   
  delay(100);
}

void handleError(char* errorText){
  Serial.println();
  Serial.println("Error: ");
  Serial.print(errorText);
  digitalWrite(D2, HIGH);
  delay(100);
  digitalWrite(D2, LOW);
  delay(100);
  digitalWrite(D2, HIGH);
  delay(100);
  digitalWrite(D2, LOW);
  delay(100);
  digitalWrite(D2, HIGH);
  delay(100);
  ESP.deepSleep(sleepMinutes * 6e7);
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(D0, WAKEUP_PULLUP);
      
  Serial.begin(9600);
  dht.begin();
  delay(200);
  
  connectWifi();
  client.setServer(mqtt_server, mqtt_port);
  
  readDhtSensor();
  publishData();
  
  ESP.deepSleep(sleepMinutes * 6e7);
}

void loop(){
}
