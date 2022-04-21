#include <Arduino.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <MQTT.h>

//WiFi
const char *ssid = "Omega-6CBE";
const char *password = "123456789"; 

WiFiClient wifiClient;
MQTTClient MQTTclient;

//MQTT Broker
const char *pubTopicInit = "/init"; //Topic to publish to
const char *pubTopic = "/loc/RSSI/send"; //Topic to publish to
const char *subTopic = "/loc/RSSI/get"; //Topic to subscribe to
const char *server = "192.168.3.1"; 

StaticJsonDocument<60> inDoc;
StaticJsonDocument<120> outDoc;

//BLE
int scanTime = 2; 
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

//Receiving Message

void messageReceived(String topic, String payload) {

  deserializeJson(inDoc, payload);

  Serial.println("Message Received:");

  const char *MAC = inDoc["MAC"];
  String ID = inDoc["ID"];
  BLEAddress target(MAC);

  Serial.println(MAC);
  Serial.println(ID);

  //Initate the Scan 

  Serial.printf("Start BLE scan for %d seconds...\n", scanTime); //Print length of scan
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false); //Starting new scan
  BLEAdvertisedDevice cur; 
  int RSSI;
 
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

  
  char output[128];

  outDoc["RSSI"] = RSSI;
  outDoc["MAC"] = MAC;
  outDoc["ID"] = ID;
  outDoc["Client ID"] = WiFi.macAddress();

  serializeJson(outDoc,output);

  MQTTclient.publish(pubTopic, output);

}

//Connect to MQTT

void connectMQTT() {

  const char *ID = WiFi.macAddress();
  MQTTclient.begin(server, wifiClient);
  MQTTclient.onMessage(messageReceived);

  while (!MQTTclient.connected()) {

    Serial.print("Connecting to MQTT...");

    if (MQTTclient.connect(ID) {

      Serial.println("Connected");
      MQTTclient.publish(pubTopic,"Client Connected");

      MQTTclient.subscribe(subTopic);
      Serial.print("Subcribe to: ");
      Serial.println(subTopic);

      Serial.print("Publish to: ");
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
