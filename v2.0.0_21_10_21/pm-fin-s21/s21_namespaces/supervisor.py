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


from flask import copy_current_request_context
from flask_socketio import Namespace, emit, disconnect
import queue
from s21_store.config import s21_config


class S21SupervisorNamespace(Namespace):
    def __init__(self, name_space, s21_tx_req_queue, s21_auth_blueprint, session, logger, secure):
        super().__init__(name_space)
        self.name_space_ = name_space
        self.s21_tx_req_queue_ = s21_tx_req_queue
        self.s21_auth_blueprint_ = s21_auth_blueprint
        self.session_ = session
        self.logger_ = logger
        self.secure_ = secure
        self.s21_cfg_ = s21_config.Config

    # Client sends S21_TX_REQ_API Message to the controller.
    # The controller acknowledges with a session ID and original message back to the client.
    # The controller adds the S21_TX_REQ_API Message to the Queue ->  s21_transmitter_worker_fn processes the message.
    # The s21_transmitter_worker_fn emits the response of the actual execution of the S21_TX_REQ_API back to the client.
    def on_s21_tx_req_api(self, message):
        self.session_['s21_tx_req_api_session_id'] = self.session_.get('s21_tx_req_api_session_id', 0) + 1
        self.logger_.info('Posting S21 command to the TX Thread ---> --->...')
        self.logger_.info('Sending Acknowledgement for S21_TX_REQ_API Message to the client ...')
        #
        # Put the client message in the Queue (s21_tx_req_cmd_queue) that TX thread processes.
        #
        try:
            self.s21_tx_req_queue_.put_nowait(message)
        except queue.Full:
            emit('s21_tx_req_api_nak',
                 {
                     's21_tx_req_api': message['s21_tx_req_api'],
                     's21_tx_req_api_nak': 'S21_TX_REQ_API NAC Message',
                     's21_tx_req_api_session_id': self.session_['s21_tx_req_api_session_id']
                 },
                 namespace=self.name_space_)

        emit('s21_tx_req_api_ack',
             {
                 's21_tx_req_api': message['s21_tx_req_api'],
                 's21_tx_req_api_ack': 'S21_TX_REQ_API ACK Message',
                 's21_tx_req_api_session_id': self.session_['s21_tx_req_api_session_id']
             },
             namespace=self.name_space_)

    # Client "disconnect" event
    def on_s21_client_disconnect_api(self):
        @copy_current_request_context
        def s21_disconnect():
            disconnect()

        # for this emit we use a callback function
        # when the callback function is invoked we know that the message has been
        # received and it is safe to disconnect
        emit('s21_client_disconnect_api_resp',
             {
                 'data': 'Disconnected!'
             },
             namespace=self.name_space_,
             callback=s21_disconnect)

    def on_s21_supervisor_ping(self):
        emit('s21_supervisor_pong', namespace=self.name_space_)

    def on_ping(self):
        emit('pong', namespace=self.name_space_)

    # Client calls ACK callback upon receiving a message from the server.
    def s21_supervisor_hello_api_ack(self):
        self.logger_.info('Client received Hello ACK message --> --> S21_SUPERVISOR_HELLO_ACK')

    # Client sends Hello Message once a successful connection is established.
    def on_s21_supervisor_hello_api(self, message):
        self.logger_.info('Sending Hello Acknowledgement Message to the client ...')
        self.session_['s21_tx_req_api_session_id'] = self.session_.get('s21_tx_req_api_session_id', 0) + 1
        emit('s21_supervisor_hello_api_ack',
             {
                 's21_supervisor_hello_api': message['s21_supervisor_hello_api'],
                 's21_supervisor_hello_api_ack': 'Client Hello ACK Message',
                 's21_tx_req_api_session_id': self.session_['s21_tx_req_api_session_id']
             },
             namespace=self.name_space_,
             callback=self.s21_supervisor_hello_api_ack)

    # Client "connect" event
    def on_connect(self):
        if self.secure_:
            if not self.s21_auth_blueprint_.session.authorized or self.s21_auth_blueprint_.token is None:
                disconnect(namespace=self.name_space_)
                return

        emit('s21_supervisor_connect_response',
             {
                 'data': 'Connected',
                 'count': 0
             },
             namespace=self.name_space_)

    # Client "disconnect" event
    def on_disconnect(self):
        pass
