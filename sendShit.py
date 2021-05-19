import paho.mqtt.client as mqtt

MATRICULA = "170109208"
REGISTER_TOPIC = "fse2020/" + MATRICULA + "/dispositivos"
LED_TOPIC = "fse2020/" + MATRICULA + "/sala/led"
TEMPERATURE_TOPIC = "fse2020/" + MATRICULA + "/+/temperatura"
HUMIDITY_TOPIC = "fse2020/" + MATRICULA + "/+/umidade"
STATE_TOPIC = "fse2020/" + MATRICULA + "/+/estado"

client = mqtt.Client()
client.connect("test.mosquitto.org")
client.loop_start();

x = '{"room": "sala"}'

client.publish(LED_TOPIC, x)
