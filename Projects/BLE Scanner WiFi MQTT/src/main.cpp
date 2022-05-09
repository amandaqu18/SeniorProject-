#include <Arduino.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <MQTT.h>
#include <vector>

//WiFi
#ifdef OREKHOV
  const char *ssid = "800Shepard";
  const char *password = "Texas2021";
  const char *server = "10.0.0.202"; 
#elif BLAKE
  const char *ssid = "Omega-6CBE";
  const char *password = "123456789";
  const char *server = "192.168.3.1"; 
#else
  const char *ssid = "Omega-6805"; 
  const char *password = "123456789";
  const char *server = "192.168.3.1"; 
#endif

char ident[20];

WiFiClient wifiClient;
MQTTClient MQTTclient;

//MQTT Broker
const char *pubTopicInit = "/init"; //Topic to publish to
const char *pubTopic = "/loc/RSSI/send"; //Topic to publish to
const char *subTopic = "/loc/RSSI/get"; //Topic to subscribe to

StaticJsonDocument<60> inDoc;
StaticJsonDocument<120> outDoc;

//BLE
int scanTime = 1; 
BLEScan* pBLEScan;

//Connect to the Wi-Fi

void connectWifi() {

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.println("");
  Serial.print("Connecting to WiFi..");

  while (WiFi.status() != WL_CONNECTED) {

    Serial.print(".");
    delay(1000);

  }

  Serial.println("Connected");

}

//Set the Scanner

void setScanner() {

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //Create new scan
  pBLEScan->setActiveScan(true); //Active scan uses more power, but get results faster
  pBLEScan->setInterval(100); //Set Scan interval
  pBLEScan->setWindow(99);  //Less or equal setInterval value

}

//Receiving Message

void messageReceived(String topic, String payload) {

  deserializeJson(inDoc, payload);

  Serial.println("");
  Serial.println("Message Received");

  const char *MAC = inDoc["MAC"];
  int ID = inDoc["ID"];
  BLEAddress target(MAC);

  //Initate the Scan 

  std::vector<int> RSSI_VALUES;
  int length = 0;
  int sum = 0;
  int AVERAGE_RSSI = 0;

  Serial.println("Starting BLE scan for 5 RSSI values...");

  while(length < 5) {

  BLEScanResults foundDevices = pBLEScan->start(scanTime, false); //Starting new scan
 
    for (int i = 0; i < foundDevices.getCount(); i++) { //Cycle through all devices found
      
       BLEAdvertisedDevice cur = foundDevices.getDevice(i); //Current device

       if (cur.getAddress().equals(target)) {  //Compare current device adress to target address

            Serial.printf("Address: %s, RSSI: %d", cur.toString().c_str(), cur.getRSSI());
            Serial.println(); //Print out address, RSSI, and TXPower of found device
            RSSI_VALUES.push_back(cur.getRSSI());
            length++;
       } 
    } 
  }

  Serial.println("Scan done!");
  pBLEScan->clearResults();   //Delete results fromBLEScan buffer to release memory

  //Get sum of all RSSI values in array

  for (int i = 0; i < length; i++) {

      sum += RSSI_VALUES[i];

  } 

  Serial.print("Sum: ");
  Serial.println(sum);

  //Average sum of RSSI values

  AVERAGE_RSSI =  sum / length;

  Serial.print("Average: ");
  Serial.println(AVERAGE_RSSI);

  char output[128];

  outDoc["RSSI"] = AVERAGE_RSSI;
  outDoc["MAC"] = MAC;
  outDoc["ID"] = ID;
  outDoc["Client ID"] = WiFi.macAddress();

  serializeJson(outDoc,output);

  MQTTclient.publish(pubTopic, output);
  Serial.println("Message Sent");

}

//Connect to MQTT

void connectMQTT() {

  MQTTclient.begin(server, wifiClient);
  MQTTclient.onMessage(messageReceived);

  while (!MQTTclient.connected()) {

    Serial.println("");
    Serial.print("Connecting to MQTT...");
    Serial.print(ident);
    Serial.print("...");

    if (MQTTclient.connect(ident)) {

      Serial.println("Connected");
      MQTTclient.publish(pubTopicInit, ident);

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

  strcpy(ident,WiFi.macAddress().c_str());
  Serial.println(ident);

  //MQTT
  connectMQTT();

  //Scanner
  setScanner();

} 

void loop() {

  if (WiFi.status() != WL_CONNECTED) {

      connectWifi();

  }

  if (!MQTTclient.connected()) {

      connectMQTT();
  }

  MQTTclient.loop();

}