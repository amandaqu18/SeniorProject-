import threading
import json

import paho.mqtt.client as mqtt
# mqtt docs:
# https://www.eclipse.org/paho/index.php?page=clients/python/docs/index.php
# Install github on Omega  Onion 2+
# https://docs.onion.io/omega2-docs/installing-and-using-git.html
# Distance Code
# https://medium.com/beingcoders/convert-rssi-value-of-the-ble-bluetooth-low-energy-beacons-to-meters-63259f307283


# START CONSTANTS
RSSI_TOPIC = "/loc/RSSI/get"
LOCATION_TOPIC = "/loc/RSSI/send"
TERMINAL_LOC = "/loc/pos"
# update to ommega onion
MQTT_URL = "192.168.3.1"
MQTT_PORT = 1883
BROADCAST_INTERVAL = 5
MEASURED_POWER = -40.0
N = 3.0
X_MAX = 6.0
Y_MAX = 4.0
CLIENT_LOCATION = {
    # x, y
    "34:AB:95:40:50:B0": [0,0],
    "08:3A:F2:3F:B5:68": [0,Y_MAX],
    "08:3A:F2:3F:D7:8C": [X_MAX,0],
    "08:3A:F2:3F:D3:48": [X_MAX,Y_MAX]
}

RSSI_COMBOS = [[0,1,2], [0,1,3], [0,2,3], [1,2,3]]
MAC_ADDR = "2C:F7:F1:1B:B7:1B"
# END CONSTANTS

# GLOBAL VARIABLES
rssi_values = {}
iter_id = 0
# END GLOBAL VARIABLES

# connection callback
def on_connect(client, userdata,flags, rc):
    print("Connected with result code: "+str(rc))
    client.subscribe(LOCATION_TOPIC)

# message callback
def on_message(client, userdata, message):
    global rssi_values
    if message.topic == LOCATION_TOPIC:
       payload = json.loads(message.payload.decode())
       print("payload id: " + str(payload['ID']) + " iter_id: " + str(iter_id))

       # if the id isn't equal to the current iter_id then its an
       # old value so discard it
       #if payload['ID'] != str(iter_id):
       #    return False

       rssi_values[payload["Client ID"]] = payload["RSSI"]

def publish(client, topic, payload):
    client.publish(topic,payload, qos=0, retain=False )

# infnite recursion every N seconds AKA broadcast loop
def ask_for_rssi(client):
    global rssi_values , iter_id
    print("PUBLISHING!")

    # new round so increment the iter_id
    iter_id = iter_id + 1

    # create the json payload to SEND to the clients
    payload = {}
    payload['ID'] = iter_id
    payload['MAC'] = MAC_ADDR

    # broadcard a request for rssi values
    publish(client, RSSI_TOPIC, json.dumps(payload))

    # wait for the clients to respond
    xy = calculate_location()

    print("RECEIVED:")
    print(xy)

    # only send the location if we have one
    if xy is not None:
        # TODO: send the x,y to WT
        xVal = xy[0]
        yVal = xy[1]

        # by calling publish()
        payload = {}
        payload['X'] = int((xVal/X_MAX)*360)
        payload['Y'] = int((yVal/Y_MAX)*240)
        payload['MAC'] = MAC_ADDR
        publish(client, TERMINAL_LOC, json.dumps(payload))
        print("XY >>>>>>>")
        print(xy)
        rssi_values = {}

    # set a 2 second delpy before broadcast another request
    threading.Timer(BROADCAST_INTERVAL, ask_for_rssi, [client]).start()

# rssi calculation function
def calculate_location():
    global rssi_values
    if len(rssi_values) < 3:
        print("not enough values " + str(len(rssi_values)))
        return None

    # client id : rssi signal
    print("rssi_values len: " + str(len(rssi_values)))

    # create an array to store the location values
    locations = []

    # calculate the location for all the rssi values we have
    for client_id in rssi_values:
        # get the x, y values of the client
        x = CLIENT_LOCATION[client_id][0]
        y = CLIENT_LOCATION[client_id][1]

        # calculate the radius (distance)
        r = distance(rssi_values[client_id])

        locations.append([x,y,r])
        print[x]
        print[y]
        print[r]
        print("----------")

    if len(rssi_values) == 3:
        print("3 values")
        return getxy(locations[0], locations[1], locations[2])
    if len(rssi_values) == 4:
        print("4 values")
        # create an array to store the xy values
        xys = []

        # loop over every combo and add it to the xys array
        # location will have 4 x,y,r values in it
        for c in RSSI_COMBOS:
            xys.append(getxy(locations[c[0]], locations[c[1]], locations[c[2]]))
        x = 0
        y = 0
        # get the average x and y
        for xy in xys:
            x = x + xy[0]
            y = y + xy[1]

        return [x/4, y/4]

    # if we get here there is too many values in the rssi_values array
    print("TO MANY VALUES: " + str(len(rssi_values)))
    exit(1)



def distance(rssi):
    distance = 10 ** ((MEASURED_POWER - rssi) / (10 * N))
    return distance


# Trilateration equation (simlified one)
# a,b,c are 3 value arrays [x,y,r]
# a = [x1, y1 ,r1]
# b = [x2, y2, r2]
# c = [x3, y3, r3]
def getxy(a,b,c):
  A = 2*b[0] - 2*a[0]
  B = 2*b[1] - 2*a[1]
  C = a[2]**2 - b[2]**2 - a[0]**2 + b[0]**2 - a[1]**2 + b[1]**2
  D = 2*c[0] - 2*b[0]
  E = 2*c[1] - 2*b[1]
  F = b[2]**2 - c[2]**2 - b[0]**2 + c[0]**2 - b[1]**2 + c[1]**2
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

# what blake needs
# SS ID, password
