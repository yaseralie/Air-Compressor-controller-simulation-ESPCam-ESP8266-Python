//Using ESP-01 as air compressor controller
//Using GPIO-0 as output

//Setup for MQTT and WiFi============================
#include <ESP8266WiFi.h>
//Library for MQTT:
#include <PubSubClient.h>

//declare topic for publish message
const char* topic_pub = "compressor1";
//declare topic for subscribe message
const char* topic_sub = "gauge1";

// Update these with values suitable for your network.
const char* ssid = "SSID";
const char* password = "PASSWORD";
const char* mqtt_server = "103.221.131.110";

//for output
int relay = 0; //relay to rurn on air compressor
float max_pressure = 90;
float min_pressure = 60;
const char* status_compressor="OFF";

WiFiClient espClient;
PubSubClient client(espClient);
//char msg[50];

void setup_wifi() {
  delay(100);
  // We start by connecting to a WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length)
{
  //Receiving message as subscriber
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  String msg_received;
  Serial.print("Pressure Received:");
  for (int i = 0; i < length; i++) {
    msg_received += ((char)payload[i]);
    //Serial.print((char)payload[i]);
  }
  float pressure = msg_received.toFloat();
  Serial.println(pressure);
  //If pressure over than max standard, turn off relay
  if (pressure > max_pressure)
  {
    digitalWrite(relay, HIGH);
    status_compressor = "OFF";
  }
  //If pressure less than max standard
  if (pressure < max_pressure)
  {
    //if pressure less than min pressure, turn on relay
    if (pressure < min_pressure)
    {
      digitalWrite(relay, LOW);
      status_compressor = "ON";
    }
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    //if you MQTT broker has clientID,username and password
    //please change following line to    if (client.connect(clientId,userName,passWord))
    if (client.connect(clientId.c_str()))
    {
      Serial.println("connected");
      //once connected to MQTT broker, subscribe command if any
      client.subscribe(topic_sub);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  //subscribe topic
  client.subscribe(topic_sub);
  //setup pin output
  pinMode(relay, OUTPUT);

  //Reset relay
  digitalWrite(relay, HIGH);
}

void loop() {
  delay(500);
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (client.publish(topic_pub, status_compressor) == true) {
    Serial.println("Success sending message");
  } else {
    Serial.println("Error sending message");
  }
  Serial.println("-------------");
}
