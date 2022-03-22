# import required libraries
from pyModbusTCP.client import ModbusClient
import paho.mqtt.publish as publish
import json
import logging
import time
import threading
import sys

def read_register(client, register, mqtt_details, packet, logger):
    try:
        while True:
            #threading.Timer(2.0, read_register(client, register, mqtt_details, packet, logger)).start()

            if client.is_open():
                # read holding value at that particular register
                regs = client.read_holding_registers(register)

                # check register value
                if regs[0] == 1:
                    temp = {'detect': "True"}

                else:
                    temp = {'detect': "False"}

                # append register value to the packet
                data = packet.update(temp)
                print(data)
                print(packet)
                # publish packet to mqtt broker
                publish.single(mqtt_details['mqtt_topic'], json.dumps(data), hostname=mqtt_details['mqtt_broker_ip'],
                               port=mqtt_details['mqtt_broker_port'], client_id=mqtt_details['mqtt_client_id'])
                print("done")
            else:
                logger.error("Client is closed")
            time.sleep(2)

    except Exception as e:
        logger.error(e)


def initialize():
    try:
        # create logging file and configure logger
        logger = logging.getLogger(__name__)
        logger.setLevel(logging.DEBUG)
        log = logging.FileHandler('logfile.log')
        log.setFormatter(logging.Formatter("%(asctime)s [%(levelname)s] %(message)s", "%Y-%m-%d %H:%M:%S"))
        logger.addHandler(log)

        # read config.json file to read configuration details
        config_file = open('config.json')
        details = json.load(config_file)
        modbus_ip = details['connection']['modbus']['modbus_server_ip']
        modbus_port = details['connection']['modbus']['modbus_server_port']
        mqtt_details = details['connection']['mqtt']
        register = details['register']['val']

        # read packet.json
        packet_file = open('packet.json')
        packet = json.load(packet_file)

        # create modbus client and connect
        client = ModbusClient(host=modbus_ip, port=modbus_port)
        client.open()

        read_register(client, register, mqtt_details, packet, logger)
        return 1
    except Exception as e:
        logger.error(e)


if __name__ == '__main__':
    ret = initialize()
