#include <Arduino.h>

#include "sys/time.h"
#include "rpcBLEDevice.h"
#include "BLEBeacon.h"
#include "TFT_eSPI.h"
#include "rpcWiFi.h"
#include <Seeed_Arduino_FS.h>
#include <Seeed_mbedtls.h>
#include <PubSubClient.h>

#define BEACON_UUID           "8ec76ea3-6668-48da-9866-75be8bc86f4d" // UUID 1

const char* ssid = "800Shepard";
const char* password =  "Texas2021";

const char *ID = "Wio-Terminal-Client";  // Name of our device, must be unique
const char *pubTopic = "myfirst/test";  // Topic to publish to
const char *subTopic = "mysecond/test";  // Topic to subcribe to
const char *server = "10.0.0.203"; 

WiFiClient wifiClient;
PubSubClient client(wifiClient);

BLEAdvertising *pAdvertising;

TFT_eSPI tft;

void callback(char* topic, byte* payload, unsigned int length) {

  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {

    Serial.print((char)payload[i]);

  }

  Serial.println();
}

void reconnect() {

  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting connection...");
    // Attempt to connect
    if (client.connect(ID)) {

      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(pubTopic, "Wio Terminal is connected!");
      Serial.println("Published connection message successfully!");
      // ... and resubscribe
      client.subscribe(subTopic);
      Serial.print("Subcribed to: ");
      Serial.println(subTopic);

    }
    else {

      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      // Wait 2 seconds before retrying
      delay(2000);

    }
  }
}

void setBeacon() {

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

}

 void setLCD(){

  tft.begin();
  tft.fillScreen(TFT_BLACK);
  tft.setRotation(3);
  tft.setTextSize(2);

 }
 

void setup() {

  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  //Create the BLE Device
  BLEDevice::init("");
  pAdvertising = BLEDevice::getAdvertising();
  //Create Scan
  setBeacon();
  // Start advertising
  pAdvertising->start();

  setLCD();

  tft.setTextColor(TFT_YELLOW);
  tft.setCursor((320 - tft.textWidth("Connecting to Wi-Fi.."))/2, 100);
  tft.print("Connecting to Wi-Fi..");

  client.setServer(server, 1883);
  client.setCallback(callback);


}

void loop() {

  //Connect to access point
  WiFi.begin(ssid, password);
  if (WiFi.status() != WL_CONNECTED) {

        WiFi.begin(ssid, password);

  }

  tft.fillScreen(TFT_BLACK);

  tft.fillCircle(160,120,20,TFT_BLUE);

  tft.setTextColor(TFT_GREEN);
  tft.setCursor((320 - tft.textWidth("Connected!"))/2, 80);
  tft.print("Connected!");

  tft.setTextColor(TFT_WHITE);
  tft.setCursor((200 - tft.textWidth("IP Address: "))/2, 120);
  tft.print("IP Address: ");
  tft.println(WiFi.localIP()); // prints out the device's IP address
  
  //Advertise BLE signal 
  if (WiFi.status() == WL_CONNECTED) {

        pAdvertising->start();

  }

  //Connect to broker
  if (!client.connected()) {

    reconnect();

  }
  client.loop();


}
