#include <Arduino.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <WiFi.h>
#include <MQTT.h>

//WiFi
const char *ssid = "800Shepard";
const char *password = "Texas2021"; 

WiFiClient wifiClient;

//BLE 
BLEAddress target("2C:F7:F1:1B:B7:1b");
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

// Initate the Scan 

void scan(){

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

  

}

void setup() {

  Serial.begin(115200);
  
  //Wifi
  connectWifi();

  //Scanner
  setScanner();

} 

void loop() {

  scan(); 

}
