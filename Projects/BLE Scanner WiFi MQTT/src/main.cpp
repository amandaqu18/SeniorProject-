#include <Arduino.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <MQTT.h>

//WiFi
const char *ssid = "800Shepard";
const char *password = "Texas2021"; 

WiFiClient wifiClient;
MQTTClient MQTTclient;

//MQTT Broker
const char *ID = "ESP32-Gateway-Client"; //Name of device
const char *pubTopic = "/loc/RSSI/send"; //Topic to publish to
const char *subTopic = "/loc/RSSI/get"; //Topic to subscribe to
const char *server = "10.0.0.210"; 

StaticJsonDocument<60> inDoc;
StaticJsonDocument<120> outDoc;

//BLE 
BLEAddress target("2C:F7:F1:1B:B7:1B");
int scanTime = 2; 
int RSSI; //Necessary for most acurate RSSI
BLEScan* pBLEScan;


//Connect to the Wi-Fi

void connectWifi() {

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi..");

  while (WiFi.status() != WL_CONNECTED) {

    Serial.print(".");
    delay(1000);

  }

  Serial.println("Connected");

}

//Set the Scanner

void setScanner(){

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //Create new scan
  pBLEScan->setActiveScan(true); //Active scan uses more power, but get results faster
  pBLEScan->setInterval(100); //Set Scan interval
  pBLEScan->setWindow(99);  //Less or equal setInterval value


}

// Initate the Scan 

void scan(){

  Serial.printf("Start BLE scan for %d seconds...\n", scanTime); //Print length of scan
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false); //Starting new scan
  BLEAdvertisedDevice cur; 
 
  for (int i = 0; i < foundDevices.getCount(); i++) { //Cycle through all devices found
      
      cur = foundDevices.getDevice(i); //Current device

    if (cur.getAddress().equals(target)) {  //Compare current device adress to target address
      
       RSSI = cur.getRSSI();
       Serial.printf("Address: %s \t RSSI: %d", cur.toString().c_str(), RSSI);
       Serial.println(); //Print out address, RSSI, and TXPower of found device
  
    } 

  } 

  Serial.println("Scan done!");
  pBLEScan->clearResults();   //Delete results fromBLEScan buffer to release memory

}

//Receiving Message

void messageReceived(String topic, String payload) {

  deserializeJson(inDoc, payload);

  Serial.println("Message Received:");

  String MAC = inDoc["MAC"];
  String ID = inDoc["ID"];

  Serial.println(MAC);
  Serial.println(ID);
  
  char output[128];

  scan();

  outDoc["RSSI"] = RSSI;
  outDoc["MAC"] = MAC;
  outDoc["ID"] = ID;
  outDoc["Client ID"] = WiFi.macAddress();

  serializeJson(outDoc,output);

  MQTTclient.publish(pubTopic, output);

}

//Connect to MQTT

void connectMQTT() {

  MQTTclient.begin(server, wifiClient);
  MQTTclient.onMessage(messageReceived);

  while (!MQTTclient.connected()) {

    Serial.print("Connecting to MQTT...");

    if (MQTTclient.connect(ID)) {

      Serial.println("Connected");

      MQTTclient.subscribe(subTopic);
      Serial.print("Subcribed to: ");
      Serial.println(subTopic);

      Serial.print("Published to: ");
      Serial.println(pubTopic);

    }
    else {

      Serial.print("failed...");
      Serial.println(" try again in 2 seconds");
      delay(2000);

    }
  }
}

void setup() {

  Serial.begin(115200);
  
  //Wifi
  connectWifi();

  //MQTT
  connectMQTT();

  //Scanner
  setScanner();

} 

void loop() {

  MQTTclient.loop();

}
