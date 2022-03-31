#include <Arduino.h>
#include <Arduino.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <WiFi.h>
#include <PubSubClient.h>

//WiFi
const char *ssid = "xxxx"; // Enter your WiFi name
const char *password = "xxxx";  // Enter WiFi password

// MQTT Broker
const char *mqtt_broker = "broker.emqx.io";
const char *topic = "esp32/test";
const char *mqtt_username = "emqx";
const char *mqtt_password = "public";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

//BLE 
BLEAddress target("2C:F7:F1:1B:B7:1b");
int scanTime = 2; 
BLEScan* pBLEScan;

void setup() {
  Serial.begin(115200);
  
  //BLE Scan
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //Create new scan
  pBLEScan->setActiveScan(true); //Active scan uses more power, but get results faster
  pBLEScan->setInterval(100); //Set Scan interval
  pBLEScan->setWindow(99);  //Less or equal setInterval value

  //WiFi/Broker
   WiFi.begin(ssid, password);
   while (WiFi.status() != WL_CONNECTED) {
     delay(500);
     Serial.println("Connecting to WiFi..");
    }

 Serial.println("Connected to the WiFi network");
 //connecting to a mqtt broker
 client.setServer(mqtt_broker, mqtt_port);
 client.setCallback(callback);
 while (!client.connected()) {
     String client_id = "esp32-client-";
     client_id += String(WiFi.macAddress());
     Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
     if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
         Serial.println("Public emqx mqtt broker connected");
     } else {
         Serial.print("failed with state ");
         Serial.print(client.state());
         delay(2000);
     }

    //Publish and Subscribe 
    client.publish(topic, "Hi EMQX I'm ESP32 ^^");
    client.subscribe(topic);

  }

} 

void callback(char *topic, byte *payload, unsigned int length) {

 Serial.print("Message arrived in topic: ");
 Serial.println(topic);
 Serial.print("Message:");

 for (int i = 0; i < length; i++) {
     Serial.print((char) payload[i]);
 }
 Serial.println();
 Serial.println("-----------------------");

}

void loop() {

  Serial.printf("Start BLE scan for %d seconds...\n", scanTime); //Print length of scan
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false); //Starting new scan
 
  for (int i = 0; i < foundDevices.getCount(); i++) { //Cycle through all devices found

     BLEAdvertisedDevice cur = foundDevices.getDevice(i); //Current device

    if (cur.getAddress().equals(target)) {  //Compare current device adress to target address
        
       Serial.printf("Address: %s \t RSSI: %d \t TXPower: %d", cur.toString().c_str(), cur.getRSSI(), cur.getTXPower());
       Serial.println(); //Print out address, RSSI, and TXPower of found device
  
    } 

  }

  Serial.println("Scan done!");
  pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory
  delay(2000);

  client.loop();

}
