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

"""
# Some useful code semantics.
# pm-fin-s21 -> Name of the solution
# s21 is a short form of pm-fin-s21
"""
from gevent import monkey

monkey.patch_all()

import os
import logging
import queue
import json
import requests
from s21_store.config import s21_config
from logging.config import dictConfig
from threading import Lock
from flask import Flask, render_template, session, redirect, url_for, abort
from flask_socketio import SocketIO
from engineio.payload import Payload
from flask import has_request_context, request
from flask.logging import default_handler
from flask_dance.consumer import OAuth2ConsumerBlueprint, oauth_authorized
from flask_login import LoginManager
from flask_cors import CORS
from werkzeug.exceptions import BadRequest, NotFound, Unauthorized
from s21_mqtt.mqtt_wss_client import S21MqttWssClient
from s21_namespaces.supervisor import S21SupervisorNamespace
from s21_namespaces.site import S21SiteNamespace
from s21_namespaces.s21global import S21GlobalNamespace
from s21_store.sys.topic_to_namespace_mappings import S21TopicToNamespaceMapper
from s21_store.sites.s21_sites import AllowedSites

# https://github.com/miguelgrinberg/python-engineio/issues/142
Payload.max_decode_packets = 5000

# Set this variable to "threading", "eventlet" or "gevent" to test the
# different async modes, or leave it set to None for the application to choose
# the best option based on installed packages.
async_mode_ = None

# Log file run/logs/pm-fin-s21.log
app_root_ = os.path.dirname(os.path.abspath(__file__))
log_dir_ = os.path.join(app_root_, 'run/logs')
if not os.path.exists(log_dir_):
    os.mkdir(log_dir_)
log_file_ = os.path.join(log_dir_, 'pm-fin-s21.log')

# https://flask.palletsprojects.com/en/2.0.x/logging/
dictConfig({
    'version': 1,
    'disable_existing_loggers': False,
    'formatters':
        {
            'standard':
                {
                    'format': '[%(asctime)s] %(levelname)s in %(module)s: %(message)s'
                }
        },
    'handlers':
        {
            'pm-fin-s21':
                {
                    'level': 'DEBUG',
                    'class': 'logging.handlers.RotatingFileHandler',
                    'filename': log_file_,
                    'formatter': 'standard',
                    'maxBytes': 10000000,
                    'backupCount': 10
                }
        },
    'loggers':
        {
            'pm-fin-s21-logger':
                {
                    'handlers': ['pm-fin-s21'],
                    'level': 'DEBUG',
                    'propagate': True
                }
        },
    'root':
        {
            'level': 'DEBUG',
            'handlers': ['pm-fin-s21']
        }
})


# More information about the request, such as the IP address, may help debugging some errors.
class RequestFormatter(logging.Formatter):
    def format(self, record):
        if has_request_context():
            record.url = request.url
            record.remote_addr = request.remote_addr
        else:
            record.url = None
            record.remote_addr = None

        return super().format(record)


formatter = RequestFormatter(
    '[%(asctime)s] %(remote_addr)s requested %(url)s\n'
    '%(levelname)s in %(module)s: %(message)s'
)

default_handler.setFormatter(formatter)
root = logging.getLogger('pm-fin-s21')
root.addHandler(default_handler)

# APP Name
app_ = Flask(__name__)
app_.config['SECRET_KEY'] = 'my_secret_pm-fin-s21_key'

#  initialise logger
logger_ = app_.logger

# OIDC and User Auth
login_manager = LoginManager()
login_manager.init_app(app_)
login_manager.login_view = 'login'
login_manager.session_protection = "strong"

os.environ["OAUTHLIB_INSECURE_TRANSPORT"] = "1"
s21_cfg_ = s21_config.Config
s21_auth_blueprint_ = OAuth2ConsumerBlueprint(
    "pm_fin_s21_auth", __name__,
    client_id=s21_cfg_['auth']['client_id_'],
    client_secret=s21_cfg_['auth']['client_secret_'],
    base_url=s21_cfg_['auth']['base_url_'],
    token_url=s21_cfg_['auth']['token_url_'],
    authorization_url=s21_cfg_['auth']['authorization_url_'],
    redirect_to="load_profile",
    authorized_url="authorized",
    login_url="login",
    scope=['openid', 'profile', 'email']
)
app_.register_blueprint(s21_auth_blueprint_, url_prefix="/login")

# Create Socket I/O object.
socketio = SocketIO(app_, async_mode=async_mode_, cors_allowed_origins="*")
CORS(app_, cors_allowed_origins="*")

print('Runnng IoT initiated financial transaction application ...')

# For s21 RX, TX Threads. The threads are created upon the first connection.
s21_receiver_thread_ = None
s21_receiver_thread_lock_ = Lock()

# Create this thread for receiving MQTT messages.
s21_mqtt_client_thread_ = None
s21_mqtt_client_thread_lock_ = Lock()

s21_transmitter_thread_ = None
s21_transmitter_thread_lock_ = Lock()

# Queue for S21 TX Messages. Used for API/Client Messages.
s21_tx_req_queue_ = queue.Queue()

# Queue for S21 RX Messages. Used by MQTT Client.
s21_rx_event_queue_ = queue.Queue()


# Thread Function to run MQTT Client
def s21_mqtt_client_worker_fn():
    while True:
        s21_mqtt_client = S21MqttWssClient(s21_rx_event_queue_, logger_)
        s21_mqtt_client.run()  # The client look shall not return.
        print('s21_mqtt_client_worker_fn')
        socketio.sleep(3)  # Just some sleep! Some messages may be lost.


# Client calls ACK callback upon receiving a message from the server.
def s21_rx_event_ack():
    logger_.info('Client received a S21 RX message S21_RX_EVENT --> --> S21_RX_EVENT_ACK')


# This thread emits the receive message over S21 MQTT channel to the clients.
def s21_receiver_worker_fn():
    # Do some stuff
    while True:
        try:
            s21_rx_event_msg = s21_rx_event_queue_.get()
            topic = s21_rx_event_msg.topic
            jm_in = json.loads(s21_rx_event_msg.payload)
            site_ncap_id = jm_in['ncap_id']
            #
            # Process the S21 RX Message over MQTT. Call FIN APIs
            #
            # Based on the S21 RX Message, the worker decides the Namespace/Room to emit the
            # messages.
            target_emit_ctx_ = S21TopicToNamespaceMapper.get_sio_site_emit_target(topic, site_ncap_id)

            # Emit to all the clients (websockets clients)
            # Emit message to site specific room.
            # TODO define messages and document them
            socketio.emit('s21_rx_event',
                          {
                              's21_rx_event': topic,
                              's21_rx_event_data': jm_in,
                              's21_txn_context':
                                  {
                                      'status_str': 'Success',
                                      'status_code': 200
                                  }
                          },
                          namespace=target_emit_ctx_['namespace'],
                          callback=s21_rx_event_ack,  # Optional
                          broadcast=True)  # to=target_emit_ctx_['rooms']['site'])
        except queue.Empty:
            logger_.exception('s21_receiver_worker_fn Error/Exception in s21_rx_event_queue_.put_nowait(msg)')
        except:
            logger_.exception('Other Exception ... (Mostly JSON payload) while executing the TX API.')
        finally:
            pass


# Client calls ACK callback upon receiving a message from the server.
def s21_tx_req_api_resp_ack():
    logger_.info('Client received a response message for S21_TX_REQ_API! --> --> S21_TX_REQ_API_RESP_ACK')


# This thread handles S21_TX_REQ_API Messages
# The s21_transmitter_worker_fn emits the response of the actual execution of the S21_TX_REQ_API back to the client.
# The clients shall send topic -> site/api or global/api in the API data.
def s21_transmitter_worker_fn():
    # Do some stuff
    count = 0
    while True:
        count += 1
        try:
            s21_tx_req_msg = s21_tx_req_queue_.get()
            site_ncap_id = s21_tx_req_msg['ncap_id']
            topic = s21_tx_req_msg['topic']
            #
            # Process the S21 TX Message
            #

            # Based on the API Message, the worker decides the Namespace/Room to emit the messages.
            target_emit_ctx_ = S21TopicToNamespaceMapper.get_sio_site_emit_target(topic, site_ncap_id)

            # The Client shall provide NAMESPACE and ROOM to emit the responses of Async APIs.
            # Send the response back to the client(s) connected. (/namespace/room)
            # TODO define messages and document them.
            logger_.info('Sending Response Message for S21_TX_REQ_API Message to the client <--- <--- ...')
            # TODO emit appropriate message to the rooms returned by get_sio_site_emit_target
            socketio.emit('s21_tx_req_api_resp',
                          {
                              's21_tx_req_api': s21_tx_req_msg,
                              's21_tx_req_api_resp_data': count
                          },
                          namespace=s21_tx_req_msg['namespace'],
                          callback=s21_tx_req_api_resp_ack,  # Optional.
                          broadcast=True)  # to=target_emit_ctx_['rooms']['site'])
        except queue.Empty:
            pass
        except:
            logger_.exception('Other Exception ... (Mostly JSON payload) while executing the TX API.')
        finally:
            pass


#################
# Insert Controller namespace (s21_bkp_ref/controller_namespace.py)
# The above file just for reference of implementing SocketIO namespaces with decorations.
#################

# app name
@app_.errorhandler(NotFound)
# inbuilt function which takes error as parameter
def not_found(e):
    return render_template("404.html", async_mode=socketio.async_mode)


# app name
@app_.errorhandler(BadRequest)
def bad_request(e):
    return 'bad request!', 400


# app name
@app_.errorhandler(Unauthorized)
def unautrorized_request(e):
    return 'unauthorized request!', 401


@login_manager.user_loader
def load_user(user_id):
    return session.get(user_id)


@oauth_authorized.connect
def redirect_to_next_url(blueprint, token):
    # set OAuth token in the token storage backend
    blueprint.token = token
    # redirect the user to the index / page url
    return render_template('index.html', async_mode=socketio.async_mode)


@app_.route("/authorized")
def authorized():
    if not s21_auth_blueprint_.session.authorized or s21_auth_blueprint_.token is None:
        return redirect(url_for("pm_fin_s21_auth.login"))
    return render_template('index.html', async_mode=socketio.async_mode)


@app_.route("/load-profile")
def load_profile():
    if not s21_auth_blueprint_.session.authorized or s21_auth_blueprint_.token is None:
        return redirect(url_for("pm_fin_s21_auth.login"))
    # have to use full URL here because base_url is not being used
    r = s21_auth_blueprint_.session.get(s21_cfg_["auth"]["userinfo_endpoint"])
    r.raise_for_status()
    data = r.json()

    session["user_session"] = {
        'id': data['sub'],
        'name': data['name'],
        'roles': data['roles']
    }
    return redirect(url_for("index"))


@app_.route("/logout")
def logout():
    return render_template('login.html', async_mode=socketio.async_mode)


@app_.route('/login')
def login():
    return render_template('index.html', async_mode=socketio.async_mode)


@app_.route('/')
def index():
    return render_template('index.html', async_mode=socketio.async_mode)


@app_.route('/pub/auth/device-token', methods=['POST'])
def get_access_token():
    # handle the POST request
    if request.method == 'POST':
        user_ = request.form.get('username')
        password_ = request.form.get('password')
        payload_ = {'grant_type': s21_cfg_['auth']['grant_type_'],
                    'client_id': s21_cfg_['auth']['client_id_'],
                    'client_secret': s21_cfg_['auth']['client_secret_'],
                    'username': user_,
                    'password': password_}
        headers_ = {"Content-Type": "application/x-www-form-urlencoded"}
        try:
            r = requests.post(s21_cfg_['auth']['token_url_'], data=payload_, headers=headers_)
            if r.status_code == 200:
                return r.text, r.status_code
            else:
                abort(401)

        except requests.exceptions.Timeout:
            # Maybe set up for a retry, or continue in a retry loop
            logger_.exception('S21MqttWssClient::run Timed out Error/Exception in connecting to IDP')
        except requests.exceptions.TooManyRedirects:
            # Tell the user their URL was bad and try a different one
            logger_.exception('S21MqttWssClient::run Too Many Redirects Error/Exception in connecting to IDP')
        except requests.exceptions.HTTPError as err:
            logger_.exception('S21MqttWssClient::run HTTP Error/Exception in connecting to IDP')
        except requests.exceptions.RequestException as e:
            # catastrophic error.
            logger_.exception('S21MqttWssClient::run Other critical Error/Exception in connecting to IDP')
        finally:
            pass


@app_.route('/sites', methods=['GET'])
def get_allowed_sites():
    return json.dumps(AllowedSites)


# ########################### PROGRAM THREAD INIT ############################
# The sequence of init is important. Test well after making any changes to the init.
socketio.on_namespace(
    S21SupervisorNamespace('/supervisor', s21_tx_req_queue_, s21_auth_blueprint_, session, logger_, False))

socketio.on_namespace(
    S21SiteNamespace('/site', s21_tx_req_queue_, s21_auth_blueprint_, session, logger_, False))

socketio.on_namespace(
    S21GlobalNamespace('/global', s21_tx_req_queue_, s21_auth_blueprint_, session, logger_, False))

for ns in AllowedSites:
    socketio.on_namespace(
        S21SiteNamespace('/' + ns['ncap_id'], s21_tx_req_queue_, s21_auth_blueprint_, session, logger_, False))


# Create this thread for receiving MQTT messages.
# global s21_mqtt_client_thread_
with s21_mqtt_client_thread_lock_:
    if s21_mqtt_client_thread_ is None:
        s21_mqtt_client_thread_ = socketio.start_background_task(s21_mqtt_client_worker_fn)

# Create S21 RX, TX Threads
# S21 RX Thread handles (emits) messages received over the MQTT channel.
# global s21_receiver_thread_
with s21_receiver_thread_lock_:
    if s21_receiver_thread_ is None:
        s21_receiver_thread_ = socketio.start_background_task(s21_receiver_worker_fn)

# S21 TX Thread handles client s21_tx_req_api messages
# global s21_transmitter_thread_
with s21_transmitter_thread_lock_:
    if s21_transmitter_thread_ is None:
        s21_transmitter_thread_ = socketio.start_background_task(s21_transmitter_worker_fn)

# Main
if __name__ == '__main__':
    socketio.run(app_)
