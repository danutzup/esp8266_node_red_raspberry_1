/*****
 *  danzup esp8266  report temperature and humidity and control rgb leds mqtt raspbery pi 
 *  inspired by this project :
 https://randomnerdtutorials.com/
 *  and this project :
  A Sketch written by Barsolai Chris.
  Github: https://github.com/ChrisBarsolai 
*****/
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"
// Uncomment one of the lines bellow for whatever DHT sensor type you're using!
#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
// Change the credentials below, so your ESP8266 connects to your router
const char* ssid = "XXXXXXXXXX";
const char* password = "dXXXXXXXX";
// Change the variable to your Raspberry Pi IP address, so it connects to your MQTT broker
const char* mqtt_server = "192.168.1.111";
// Initializes the espClient. You should change the espClient name if you have multiple ESPs running in your home automation system
WiFiClient espClient;
PubSubClient client(espClient);
const int RED_PIN = 5;    //D1: Red Pin
const int GREEN_PIN = 4;  //D2: Green Pin
const int BLUE_PIN = 0;   //D3: Blue Pin
// DHT Sensor - GPIO 5 = D1 on ESP-12E NodeMCU board
//const int DHTPin = 5;
const int DHTPin = 14; // DHT Sensor connected to D5.
// Lamp - LED - GPIO 4 = D2 on ESP-12E NodeMCU board
//const int lamp = 4;
const int lamp = 12;  //pin D6
// Initialize DHT sensor.
DHT dht(DHTPin, DHTTYPE);
// Timers auxiliar variables
long now = millis();
long lastMeasure = 0;
long lastMsg = 0;
char msg[50];
int value = 0;
int LEDStatus = 0;         // current status LED
int switchPIN = 2;           // pin switch
int switchStatus;            // status switch
int switchStatusLast = HIGH;  // last status switch
// Initial state of RGB LED. It sets the RGB LED to green.
void intialState(){
      digitalWrite(RED_PIN, LOW);
      digitalWrite(GREEN_PIN, LOW);
      digitalWrite(BLUE_PIN, LOW);
      snprintf (msg, 75, "Off");
      client.publish("room/LEDstate", msg);
}
// Don't change the function below. This functions connects your ESP8266 to your router
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected - ESP IP address: ");
  Serial.println(WiFi.localIP());
}
// This functions is executed when some device publishes a message to a topic that your ESP8266 is subscribed to
// Change the function below to add logic to your program, so when a device publishes a message to a topic that 
// your ESP8266 is subscribed you can actually do something
void callback(String topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp; 
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();
  // Feel free to add more if statements to control more GPIOs with MQTT
  // Switch on Red LED if an 'R' was received as first character
  if ((char)message[0] == 'R') {
  digitalWrite(RED_PIN, HIGH);
  digitalWrite(GREEN_PIN, LOW);
  digitalWrite(BLUE_PIN, LOW);
  snprintf (msg, 75, "Red");
    client.publish("room/LEDstate", msg);
  }
  // Switch on Green LED if an 'G' was received as first character
  if ((char)message[0] == 'G') {
  digitalWrite(RED_PIN, LOW);
  digitalWrite(GREEN_PIN, HIGH);
  digitalWrite(BLUE_PIN, LOW);
  snprintf (msg, 75, "Green");
    client.publish("room/LEDstate", msg);
  }
  // Switch on Blue LED if an 'B' was received as first character
  if ((char)message[0] == 'B') {
  digitalWrite(RED_PIN, LOW);
  digitalWrite(GREEN_PIN, LOW);
  digitalWrite(BLUE_PIN, HIGH);
  snprintf (msg, 75, "Blue");
    client.publish("room/LEDstate", msg);
  }
   // Switch on  LED if an 'O' was received as first character
  if ((char)message[0] == 'O') {
  digitalWrite(RED_PIN, LOW);
  digitalWrite(GREEN_PIN, LOW);
  digitalWrite(BLUE_PIN, HIGH);
  snprintf (msg, 75, "Blue");
    client.publish("room/LEDstate", msg);
  }
   // Switch off LED if an 'S' was received as first character
  if ((char)message[0] == 'S') {
  digitalWrite(RED_PIN, LOW);
  digitalWrite(GREEN_PIN, LOW);
  digitalWrite(BLUE_PIN, LOW);
  snprintf (msg, 75, "Off");
    client.publish("room/LEDstate", msg);
  }
  // If a message is received on the topic room/lamp, you check if the message is either on or off. Turns the lamp GPIO according to the message
  if(topic=="room/lamp"){
      Serial.print("Changing Room lamp to ");
      if(messageTemp == "on"){
        digitalWrite(lamp, HIGH);
        Serial.print("On");
      }
      else if(messageTemp == "off"){
        digitalWrite(lamp, LOW);
        Serial.print("Off");
      }
  }
  Serial.println();
}

// This functions reconnects your ESP8266 to your MQTT broker
// Change the function below if you want to subscribe to more topics with your ESP8266 
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    /*
     YOU MIGHT NEED TO CHANGE THIS LINE, IF YOU'RE HAVING PROBLEMS WITH MQTT MULTIPLE CONNECTIONS
     To change the ESP device ID, you will have to give a new name to the ESP8266.
     Here's how it looks:
       if (client.connect("ESP8266Client")) {
     You can do it like this:
       if (client.connect("ESP1_Office")) {
     Then, for the other ESP:
       if (client.connect("ESP2_Garage")) {
      That should solve your MQTT multiple connections problem
    */
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");  
      // Subscribe or resubscribe to a topic
      // You can subscribe to more topics (to control more LEDs in this example)
      client.subscribe("room/LEDchange");
      // You can subscribe to more topics (to control more LEDs in this example)
      client.subscribe("room/lamp");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// The setup function sets your ESP GPIOs to Outputs, starts the serial communication at a baud rate of 115200
// Sets your mqtt broker and sets the callback function
// The callback function is what receives messages and actually controls the LEDs
void setup() {
    // drive the LED to be outputs:
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(lamp, OUTPUT);  
  pinMode(switchPIN, INPUT);
  dht.begin();
  Serial.begin(115200);
  setup_wifi();
  intialState();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

// For this project, you don't need to change anything in the loop function. Basically it ensures that you ESP is connected to your broker
void loop() {

switchStatus = digitalRead(switchPIN);   // read status of switch
if (switchStatus == LOW)  // if status of button has changed
    {
      delay(60);     // debounce time of 50 ms
      switchStatus = digitalRead(switchPIN);   // read the status of the switchPIN again
      if (switchStatus == LOW) // if the status of the button has changed again
      {
        // if switch is pressed than change the LED status
        //if (switchStatus == HIGH && switchStatusLast == LOW) 
        //LEDStatus = LEDStatus + 1;
        //digitalWrite(LEDpin, LEDStatus);  // turn the LED on or off
        delay(80);
        ++LEDStatus;
if (LEDStatus == 0)
{
      digitalWrite(RED_PIN, LOW);
      digitalWrite(GREEN_PIN, LOW);
      digitalWrite(BLUE_PIN, LOW);
      snprintf (msg, 75, "Off");
      client.publish("room/LEDstate", msg);
              //switchStatus = switchStatusLast;
}
if (LEDStatus == 1)
{
      digitalWrite(RED_PIN, HIGH);
      digitalWrite(GREEN_PIN, LOW);
      digitalWrite(BLUE_PIN, LOW);
      snprintf (msg, 75, "Red");
      client.publish("room/LEDstate", msg);
              //switchStatus = switchStatusLast;
}
if (LEDStatus == 2)
{
      digitalWrite(RED_PIN, LOW);
      digitalWrite(GREEN_PIN, HIGH);
      digitalWrite(BLUE_PIN, LOW);
      snprintf (msg, 75, "Green");
      client.publish("room/LEDstate", msg);
              //switchStatus = switchStatusLast;
}
if (LEDStatus == 3)
{
      digitalWrite(RED_PIN, LOW);
      digitalWrite(GREEN_PIN, LOW);
      digitalWrite(BLUE_PIN, HIGH);
      snprintf (msg, 75, "Blue");
      client.publish("room/LEDstate", msg);
              //switchStatus = switchStatusLast;
}
if (LEDStatus > 3){
  LEDStatus = 0;       
        //switchStatus = switchStatusLast;
      digitalWrite(RED_PIN, LOW);
      digitalWrite(GREEN_PIN, LOW);
      digitalWrite(BLUE_PIN, LOW);
      snprintf (msg, 75, "Off");
      client.publish("room/LEDstate", msg);
        
      }
    }
}
  if (!client.connected()) {
    reconnect();
  }
  if(!client.loop())
    client.connect("ESP8266Client");

  now = millis();
  // Publishes new temperature and humidity every 50 seconds
  if (now - lastMeasure > 50000) {
    lastMeasure = now;
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    float h = dht.readHumidity();
    // Read temperature as Celsius (the default)
    float t = dht.readTemperature();
    // Read temperature as Fahrenheit (isFahrenheit = true)
    float f = dht.readTemperature(true);

    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t) || isnan(f)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }

    // Computes temperature values in Celsius
    float hic = dht.computeHeatIndex(t, h, false);
    static char temperatureTemp[7];
    dtostrf(hic, 6, 2, temperatureTemp);
    
    // Uncomment to compute temperature values in Fahrenheit 
    // float hif = dht.computeHeatIndex(f, h);
    // static char temperatureTemp[7];
    // dtostrf(hic, 6, 2, temperatureTemp);
    
    static char humidityTemp[7];
    dtostrf(h, 6, 2, humidityTemp);

    // Publishes Temperature and Humidity values
    client.publish("room/temperature", temperatureTemp);
    client.publish("room/humidity", humidityTemp);
    
    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.print(" %\t Temperature: ");
    Serial.print(t);
    Serial.print(" *C ");
    Serial.print(f);
    Serial.print(" *F\t Heat index: ");
    Serial.print(hic);
    Serial.println(" *C ");
    // Serial.print(hif);
    // Serial.println(" *F");
  }
} 
