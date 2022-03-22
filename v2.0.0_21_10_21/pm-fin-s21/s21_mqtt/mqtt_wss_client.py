"""
#==========================LICENSE NOTICE==========================
#
# Copyright (c) 2021 Vidcentum R&D Pvt Ltd, India.
# License: Refer to LICENSE file of the software package.
# Email: support@vidcentum.com
# Website: https://vidcentum.com
#
##========================END LICENSE NOTICE========================
"""
import os
import requests
import paho.mqtt.client as mqtt
import queue
from s21_store.config import s21_config


class S21MqttWssClient:

    def __init__(self, pub_queue, logger):
        self.s21_rx_event_queue_ = pub_queue
        self.logger_ = logger
        self.s21_cfg_ = s21_config.Config

    # Connection success callback
    def on_connect(self, client, userdata, flags, rc, properties=None):
        print('Connected with result code ' + str(rc))

        if rc != 0:
            # Don't worry, the Application client thread will attempt the MQTT client connection.
            client.disconnect()

        try:
            client.subscribe(self.s21_cfg_['mqtt_broker']['mqtt_topics_'])
        except:
            self.logger_.exception('S21MqttWssClient::on_connect MQTT subscription Error/Exception')
            pass

    # Message receiving callback
    def on_message(self, client, userdata, msg):
        # a_msg = str(msg.payload.decode("utf-8", "ignore"))
        try:
            self.s21_rx_event_queue_.put_nowait(msg)
        except queue.Full:
            self.logger_.exception(
                'S21MqttWssClient::on_message Error/Exception in s21_rx_event_queue_.put_nowait(msg)')
        finally:
            pass

    def run(self):
        # MQTT Client Object
        client = mqtt.Client(client_id=self.s21_cfg_['mqtt_broker']['mqtt_client_id_'],
                             clean_session=True, userdata=None,
                             protocol=mqtt.MQTTv311, transport="websockets")

        dirname = os.path.dirname(__file__)
        ca_cert_filename = os.path.join(dirname, '../' + self.s21_cfg_['mqtt_broker']['ca_cert_'])
        client.tls_set(ca_cert_filename)
        client.ws_set_options(self.s21_cfg_['mqtt_broker']['mqtt_ws_path_'])

        # Specify callback function
        client.on_connect = self.on_connect
        client.on_message = self.on_message

        payload_ = {'username': self.s21_cfg_['auth']['username_'],
                    'password': self.s21_cfg_['auth']['password_']}
        headers_ = {"Content-Type": "application/x-www-form-urlencoded"}
        try:
            token_url_ = self.s21_cfg_['api_server']['base_url'] + self.s21_cfg_['api_server']['device_token_endpoint']
            r = requests.post(token_url_, data=payload_, headers=headers_)
            if r.status_code == 200:
                access_token_ = r.json()['access_token']
            else:
                access_token_ = ""
            # Now set the token to access MQTT broker.
            client.username_pw_set(username=self.s21_cfg_['auth']['username_'], password=access_token_)
        except requests.exceptions.Timeout:
            # Maybe set up for a retry, or continue in a retry loop
            self.logger_.exception('S21MqttWssClient::run Timed out Error/Exception in connecting to IDP')
        except requests.exceptions.TooManyRedirects:
            # Tell the user their URL was bad and try a different one
            self.logger_.exception('S21MqttWssClient::run Too Many Redirects Error/Exception in connecting to IDP')
        except requests.exceptions.HTTPError as err:
            self.logger_.exception('S21MqttWssClient::run HTTP Error/Exception in connecting to IDP')
        except requests.exceptions.RequestException as e:
            # catastrophic error.
            self.logger_.exception('S21MqttWssClient::run Other critical Error/Exception in connecting to IDP')
        finally:
            pass

        try:
            # Establish a connection
            client.connect(self.s21_cfg_['mqtt_broker']['mqtt_server_name_'],
                           self.s21_cfg_['mqtt_broker']['mqtt_server_port_'],
                           60)

            # Run the MQTT Client.
            client.loop_forever()
        except:
            # MQTT error.
            self.logger_.exception(
                'S21MqttWssClient::run Other critical Error/Exception connecting to MQTT broker and run time.')
        finally:
            pass
