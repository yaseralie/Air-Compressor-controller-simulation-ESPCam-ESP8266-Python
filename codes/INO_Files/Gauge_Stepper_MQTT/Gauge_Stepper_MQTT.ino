//Reference GPIO  https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/

//Setup for Stepper============================
#include <AccelStepper.h>

//Setup for MQTT and WiFi============================
#include <ESP8266WiFi.h>
//Library for MQTT:
#include <PubSubClient.h>

//declare topic for subscribe message
const char* topic_sub = "compressor1";

// Update these with values suitable for your network.
const char* ssid = "SSID";
const char* password = "PASSWORD";
const char* mqtt_server = "103.221.131.110";

WiFiClient espClient;
PubSubClient client(espClient);

const int stepsPerRevolution = 2048;  // change this to fit the number of steps per revolution

// ULN2003 Motor Driver Pins
#define IN1 5
#define IN2 4
#define IN3 14
#define IN4 12

int cw_button = 0;
int ccw_button = 2;

// initialize the stepper library
AccelStepper stepper(AccelStepper::HALF4WIRE, IN1, IN3, IN2, IN4);

String status_compressor = "";

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
  for (int i = 0; i < length; i++) {
    msg_received += ((char)payload[i]);
    //Serial.print((char)payload[i]);
  }
  status_compressor = msg_received;
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
      delay(500);
    }
  }
}

void setup() {
  // initialize the serial port
  Serial.begin(115200);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  //subscribe topic
  client.subscribe(topic_sub);

  // initialize input button (common = ground)
  pinMode(cw_button, INPUT_PULLUP);
  pinMode(ccw_button, INPUT_PULLUP);
  // set the speed and acceleration
  stepper.setMaxSpeed(20);
  stepper.setAcceleration(30);
}

void loop() {
  //delay(500);
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Read the value of the input. It can either be 1 or 0
  int cw_button_val = digitalRead(cw_button);
  int ccw_button_val = digitalRead(ccw_button);

  //command for clockwise
  if (cw_button_val == LOW) {
    status_compressor = "";
    stepper.setSpeed(500);
    stepper.runSpeed();
  }
  //command for counter clockwise
  else if (ccw_button_val == LOW) {
    status_compressor = "";
    stepper.setSpeed(-500);
    stepper.runSpeed();
  }

  else {
    //If compressor ON
    if (status_compressor == "ON") {
      stepper.setSpeed(-50);
      stepper.runSpeed();
    }
    //If compressor OFF
    if (status_compressor == "OFF") {
      stepper.setSpeed(30);
      stepper.runSpeed();
    }
  }
}
