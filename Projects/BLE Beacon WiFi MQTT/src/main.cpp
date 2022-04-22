#include <Arduino.h>

#include "sys/time.h"
#include "rpcBLEDevice.h"
#include "BLEBeacon.h"
#include "TFT_eSPI.h"
#include "rpcWiFi.h"
#include <Seeed_Arduino_FS.h>
#include <Seeed_mbedtls.h>
#include <MQTT.h>
#include <ArduinoJson.h>

#define BEACON_UUID           "8ec76ea3-6668-48da-9866-75be8bc86f4d" // UUID 1

//Wi-Fi
const char *ssid = "Omega-6CBE";
const char *password = "123456789"; 

WiFiClient wifiClient;
MQTTClient MQTTclient;

//MQTT Broker
const char *ID = "Wio-Terminal-Client";  // Name of our device, must be unique
const char *subTopic = "/loc/pos";  // Topic to publish to
const char *server = "192.168.3.1";  

StaticJsonDocument<40> inDoc;
StaticJsonDocument<40> outDoc;

//BLE
BLEAdvertising *pAdvertising;

//LCD
TFT_eSPI tft;

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

//Connect to MQTT Broker

void messageReceived(String topic, String payload) {

  deserializeJson(inDoc, payload);

  Serial.println("Message Received:");

  int X = inDoc["X"];
  int Y = inDoc["Y"];
  String MAC = inDoc["MAC"];

  Serial.println(X);
  Serial.println(Y);
  Serial.println(MAC);
  Serial.print(WiFi.macAddress());

  if(WiFi.macAddress() = MAC) {

    tft.fillScreen(TFT_BLACK);
    tft.fillCircle(X,Y,10,TFT_BLUE);

  }

}

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


    }
    else {

      Serial.print("failed...");
      Serial.println(" try again in 2 seconds");
      delay(2000);

    }
  }
}

//Set the Beacon

void setBeacon() {

  BLEDevice::init("");
  pAdvertising = BLEDevice::getAdvertising();

  BLEBeacon oBeacon = BLEBeacon();
  oBeacon.setManufacturerId(0x4C00); // fake Apple 0x004C LSB (ENDIAN_CHANGE_U16!)
  oBeacon.setProximityUUID(BLEUUID(BEACON_UUID));
  oBeacon.setMajor(0x007B);
  oBeacon.setMinor(0x01C8);

  oBeacon.setSignalPower(4); //Set Signal Power

  BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();
  BLEAdvertisementData oScanResponseData = BLEAdvertisementData();

  oAdvertisementData.setFlags(0x04); // BR_EDR_NOT_SUPPORTED 0x04

  std::string strServiceData = "";
  strServiceData += (char)26;     // Len
  strServiceData += (char)0xFF;   // Type
  strServiceData += oBeacon.getData();
  oAdvertisementData.addData(strServiceData);

  oScanResponseData.setName("wio_beacon");

  pAdvertising->setAdvertisementData(oAdvertisementData);
  pAdvertising->setScanResponseData(oScanResponseData);
  pAdvertising->setAdvertisementType(GAP_ADTYPE_ADV_SCAN_IND);

  pAdvertising->start();

}

//Initilized the LCD Setup

 void initTFT(){

  tft.begin();
  tft.fillScreen(TFT_BLACK);
  tft.setRotation(3);
  tft.setTextSize(2);

  tft.fillScreen(TFT_BLACK);
  tft.fillCircle(160,120,10,TFT_BLUE);

 }
 
void setup() {

  Serial.begin(115200);

  //Wifi
  connectWifi();
  
  //Beacon
  setBeacon();

  //MQTT
  connectMQTT();
  
  //TFT
  initTFT();

}

void loop() {

  MQTTclient.loop();

}
