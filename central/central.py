import paho.mqtt.client as mqtt

MATRICULA = "170109208"
REGISTER_TOPIC = "fse2020/" + MATRICULA + "/dispositivos"
TEMPERATURE_TOPIC = "fse2020/" + MATRICULA + "/+/temperatura"
HUMIDITY_TOPIC = "fse2020/" + MATRICULA + "/+/umidade"
STATE_TOPIC = "fse2020/" + MATRICULA + "/+/estado"

def on_connect(client, userdata, flags, rc):
	client.subscribe(REGISTER_TOPIC)
	client.subscribe(TEMPERATURE_TOPIC)
	client.subscribe(HUMIDITY_TOPIC)
	client.subscribe(STATE_TOPIC)
	
def on_message(client, userdata, msg):
    print(msg.topic + " " + str(msg.payload))

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect("test.mosquitto.org")
client.loop_forever()