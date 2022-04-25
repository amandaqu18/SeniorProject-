import threading
import json

import paho.mqtt.client as mqtt
# mqtt docs:
# https://www.eclipse.org/paho/index.php?page=clients/python/docs/index.php
# https://docs.onion.io/omega2-docs/installing-and-using-git.html
#


# START CONSTANTS
RSSI_TOPIC = "/loc/RSSI/get"
LOCATION_TOPIC = "/loc/RSSI/send"
# update to ommega onion 
MQTT_URL = "192.168.3.133"
MQTT_PORT = 1883
BROADCAST_INTERVAL = 2.0
MEASURED_POWER = 4
CLIENT_LOCATION = {
    # x, y
    "id1": [0,0],
    "id2": [0,20],
    "id3": [20,0],
    "id4": [20,20]
}
# END CONSTANTS

rssi_values = {}

# connection callback
def on_connect(client, userdata,flags, rc):
    print("Connected with result code: "+str(rc))
    client.subscribe(LOCATION_TOPIC)

# message callback
def on_message(client, userdata, message):
    global rssi_values
    if message.topic == LOCATION_TOPIC:
       payload = json.loads(message.payload.decode())  

       rssi_values[payload["ID"]] = payload["RSSI"]

       if len(rssi_values) == 3:
           xy = calculate_location(rssi_values)
           rssi_values = {}
           print("XY")
           print(xy)
           # TODO: send the x,y to WT
           # by caling publixh()

           

def publish(client, topic, payload):
    client.publish(topic,payload, qos=0, retain=False )

# infnite recursion every N seconds AKA broadcast loop
def ask_for_rssi(client):    
    print("PUBLISHING!")
    publish(client, RSSI_TOPIC, "rssi")
    threading.Timer(BROADCAST_INTERVAL, ask_for_rssi, [client]).start()

# rssi calculation functions
def calculate_location(rssis):
    # client id : rssi signal
    locations = []
    for id in rssis:
        x = CLIENT_LOCATION[id][0]
        y = CLIENT_LOCATION[id][1]
        r = distance(rssis[id])
       
        locations.append([x,y,r])

    # locations
    # [ [x1,y1,r1], [x2,y2,r2], [x3,y3,r3] ]
    # turn it into line 82 def getxy
    return getxy(*[item for sublist in locations for item in sublist])    

def distance(rssi):
    N = 1
    distance = 10 ** ((MEASURED_POWER - rssi) / (10 * N))
    return distance

# Traingulation equation (simlified one)
def getxy(x1,y1,r1,x2,y2,r2,x3,y3,r3):
  A = 2*x2 - 2*x1
  B = 2*y2 - 2*y1
  C = r1**2 - r2**2 - x1**2 + x2**2 - y1**2 + y2**2
  D = 2*x3 - 2*x2
  E = 2*y3 - 2*y2
  F = r2**2 - r3**2 - x2**2 + x3**2 - y2**2 + y3**2
  x = (C*E - F*B) / (E*A - B*D)
  y = (C*D - A*F) / (B*D - A*E)
  return x,y


def run():
    print("Location calculator starting up")

    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message

    client.connect(MQTT_URL,MQTT_PORT, 60)

    # setting up the broadcast loop
    # sending messages every n seconds
    ask_for_rssi(client) 


    client.loop_forever()



if __name__ == "__main__":
    run()