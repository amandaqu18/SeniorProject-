#include <Arduino.h>

#include "sys/time.h"
#include "rpcBLEDevice.h"
#include "BLEBeacon.h"
#include "TFT_eSPI.h"
#include "rpcWiFi.h"
#include <Seeed_Arduino_FS.h>
#include <Seeed_mbedtls.h>

#define BEACON_UUID           "8ec76ea3-6668-48da-9866-75be8bc86f4d" // UUID 1

const char* ssid = "800Shepard";
const char* password =  "Texas2021";

BLEAdvertising *pAdvertising;

TFT_eSPI tft;


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


}

void loop() {

  WiFi.begin(ssid, password);
  if (WiFi.status() != WL_CONNECTED) {

        WiFi.begin(ssid, password);

  }

  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_GREEN);
  tft.setCursor((320 - tft.textWidth("Connected!"))/2, 80);
  tft.print("Connected!");

  tft.setTextColor(TFT_WHITE);
  tft.setCursor((200 - tft.textWidth("IP Address: "))/2, 120);
  tft.print("IP Address: ");
  tft.println(WiFi.localIP()); // prints out the device's IP address
  
  if (WiFi.status() == WL_CONNECTED) {

        pAdvertising->start();

  }


}
