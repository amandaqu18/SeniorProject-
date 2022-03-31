#include <Arduino.h>

#include "sys/time.h"
#include "rpcBLEDevice.h"
#include "BLEBeacon.h"
#include "TFT_eSPI.h"

BLEAdvertising *pAdvertising;

#define BEACON_UUID           "8ec76ea3-6668-48da-9866-75be8bc86f4d" // UUID 1

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

void setup() {

  // Create the BLE Device
  BLEDevice::init("");

  // Create the BLE Server
  // BLEServer *pServer = BLEDevice::createServer(); // <-- no longer required to instantiate BLEServer, less flash and ram usage

  pAdvertising = BLEDevice::getAdvertising();

  setBeacon();
   // Start advertising
  pAdvertising->start();
  //Serial.println("Advertizing started...");
  delay(100);
  //Serial.printf("in deep sleep\n");

  tft.begin();
  tft.fillScreen(TFT_BLACK);
  tft.setRotation(3);
  tft.setTextSize(2);
  tft.setCursor((320 - tft.textWidth("BLE Beacon")) / 2, 80);
  tft.print("BLE Beacon");

  tft.setCursor((320 - tft.textWidth(BLEDevice::getAddress().toString().c_str())) / 2, 120);
  tft.print(BLEDevice::getAddress().toString().c_str());

}

void loop() {
}
