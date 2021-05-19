import paho.mqtt.client as mqtt
import curses
import json
import time

MATRICULA = "170109208"
REGISTER_TOPIC = "fse2020/" + MATRICULA + "/dispositivos/+"
TEMPERATURE_TOPIC = "fse2020/" + MATRICULA + "/+/temperatura"
HUMIDITY_TOPIC = "fse2020/" + MATRICULA + "/+/umidade"
STATE_TOPIC = "fse2020/" + MATRICULA + "/+/estado"
LED_TOPIC = "fse2020/" + MATRICULA + "/+/led"

APP_DATA = {
    "pending_devices": [],
    "devices": {},
    "alarm": 0
}

def registerPendingDevice(data):
    json_data = json.loads(data)
    
    APP_DATA['pending_devices'].append({
        "id": json_data['id']
    })

def registerDevice(device):
    # device = APP_DATA['devices'][-1]
    json_data = '{"room": \"'  + device['room'] + '\"}'
    
    client.publish("fse2020/{}/dispositivos/{}".format(MATRICULA, device['id']), json_data)
    
def toggleLED(device):
    client.publish("fse2020/{}/{}/LED".format(MATRICULA, device['room']))
    
def updateTemperature(data):
    json_data = json.loads(data)
    device = APP_DATA['devices'][json_data.id]
    
    device['temperature'] = json_data.data
    
def updateHumidity(data):
    json_data = json.loads(data)
    device = APP_DATA['devices'][json_data.id]
    
    device['humidity'] = json_data.data
    
def updateState(data):
    json_data = json.loads(data)
    device = APP_DATA['devices'][json_data.id]
    
    device['state'] = json_data.data
    
def checkInput(stdscr):
    user_input = stdscr.getch()
    
    if(user_input == 10):
        return True
    return False

def handleUserRegister(stdscr, index):
    stdscr.clear()
    
    device = APP_DATA['pending_devices'][index]
    stdscr.addstr("Insira o comodo desejado: ")
    stdscr.refresh()
    # room = input()
    room = "Fonk"
    
    APP_DATA['devices'][device["id"]] = {
        "room": room,
        "temperature": -1,
        "humidity": -1,
        "state": 0,
        "id":device["id"]
    }
    print("olar")
    registerDevice(APP_DATA['devices'][device["id"]]);
    
    del APP_DATA['pending_devices'][index]

def handleUserAction(stdscr, user_input):
    stdscr.clear()
    count = 49
    
    if(user_input == 49):
        stdscr.addstr("--DISPOSITIVOS PENDENTES--\n")
        for device in APP_DATA['pending_devices']:
            stdscr.addstr("{}. ID: {}\n\n".format(chr(count), device['id']))
            count += 1
        
        action = stdscr.getch()
        index = action - 49
        handleUserRegister(stdscr, index)
        
    else:
        stdscr.addstr("--DISPOSITIVOS--\n")
        devices = list(APP_DATA['devices'].items())
        for device in devices:
            stdscr.addstr("{}. ID: {}\nCOMODO: {}\n\n".format(chr(count), device[0], device[1]['room']))
            count += 1
        
        action = stdscr.getch()
        index = action - 49
        handleUserRegister(stdscr, index)
        toggleLED(devices[index][1])

def handleUserInput(stdscr):
    stdscr.clear()
    curses.echo()
    stdscr.nodelay(False)
    
    stdscr.addstr("==============-CHOOSE ACTION-==============\n")
    stdscr.addstr("1. Register device\n")
    stdscr.addstr("2. Toggle LED\n")
    
    stdscr.addstr("================---- -----================\n")
    
    user_input = stdscr.getch()

    handleUserAction(stdscr, user_input)
    
    curses.noecho()
    stdscr.nodelay(True)
    
def displayInformation(stdscr):
    stdscr.clear()
    
    stdscr.addstr("--DISPOSITIVOS--\n")
    for device in APP_DATA['devices'].values():
        print(APP_DATA['devices'])
        stdscr.addstr("ID: {}\nCOMODO: {}\nTEMPERATURA: {}\nUMIDADE: {}\nESTADO: {}\n\n".format(device['id'], device['room'], device['temperature'], device['humidity'], device['state']))
    
    stdscr.addstr("--DISPOSITIVOS PENDENTES--\n")
    for device in APP_DATA['pending_devices']:
        stdscr.addstr("ID: {}\n\n".format(device['id']))
        
    stdscr.addstr("Pressione Enter para acessar o menu...\n")

    
def interface_loop(stdscr):
    while(True):
        if(checkInput(stdscr)):
            handleUserInput(stdscr)
            print("dong")
            continue
            
        displayInformation(stdscr)
        time.sleep(2)

def init_interface():
    stdscr = curses.initscr()
    curses.noecho()
    curses.cbreak()
    stdscr.nodelay(True)
    
    interface_loop(stdscr)
    

def init_client():
    
    client.on_connect = on_connect
    client.on_message = on_message

    client.connect("test.mosquitto.org")
    client.loop_start()
    
    return client

def on_connect(client, userdata, flags, rc):
	client.subscribe(REGISTER_TOPIC)
	client.subscribe(TEMPERATURE_TOPIC)
	client.subscribe(HUMIDITY_TOPIC)
	client.subscribe(STATE_TOPIC)
	client.subscribe(LED_TOPIC)
	
def on_message(client, userdata, msg):
    if('dispositivos' in msg.topic):
        registerPendingDevice(msg.payload)
    
    elif('temperatura' in msg.topic):
        updateTemperature(msg.payload)
    
    elif('umidade' in msg.topic):
        updateHumidity(msg.payload)
        
    elif('estado' in msg.topic):
        updateState(msg.payload)
    
    print(msg.topic + " " + str(msg.payload))

client = mqtt.Client()
init_client()
init_interface()
