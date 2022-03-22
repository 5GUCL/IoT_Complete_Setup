'''
# Client sends S21_TX_REQ_API Message to the controller.
# The controller acknowledges with a session ID and original message back to the client.
# The controller adds the S21_TX_REQ_API Message to the Queue ->  s21_transmitter_worker_fn processes the message.
# The s21_transmitter_worker_fn emits the response of the actual execution of the S21_TX_REQ_API back to the client.
@socketio.on('s21_tx_req_api', namespace='/controller')
def s21_tx_req_api_handler(message):
    session['s21_tx_req_api_session_id'] = session.get('s21_tx_req_api_session_id', 0) + 1
    logger_.info('Posting S21 command to the TX Thread ---> --->...')
    logger_.info('Sending Acknowledgement for S21_TX_REQ_API Message to the client ...')
    #
    # Put the client message in the Queue (s21_tx_req_cmd_queue) that TX thread processes.
    #
    try:
        s21_tx_req_queue_.put_nowait(message)
    except queue.Full:
        emit('s21_tx_req_api_nak',
             {
                 's21_tx_req_api': message['s21_tx_req_api'],
                 's21_tx_req_api_nak': 'S21_TX_REQ_API NAC Message',
                 's21_tx_req_api_session_id': session['s21_tx_req_api_session_id']
             },
             namespace='/controller')

    emit('s21_tx_req_api_ack',
         {
             's21_tx_req_api': message['s21_tx_req_api'],
             's21_tx_req_api_ack': 'S21_TX_REQ_API ACK Message',
             's21_tx_req_api_session_id': session['s21_tx_req_api_session_id']
         },
         namespace='/controller')


# Client "disconnect" event
@socketio.on('s21_client_disconnect_api', namespace='/controller')
def disconnect_request():
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
         namespace='/controller',
         callback=s21_disconnect)


# Client "disconnect" event
@socketio.on('disconnect', namespace='/controller')
def disconnect_request():
    pass


# Client calls ACK callback upon receiving a message from the server.
def s21_controller_hello_api_ack():
    logger_.info('Client received Hello ACK message --> --> S21_CONTROLLER_HELLO_ACK')


# Client sends Hello Message once a successful connection is established.
@socketio.on('s21_controller_hello_api', namespace='/controller')
def s21_ctrl_hello_api_handler(message):
    logger_.info('Sending Hello Acknowledgement Message to the client ...')
    session['s21_tx_req_api_session_id'] = session.get('s21_tx_req_api_session_id', 0) + 1
    emit('s21_controller_hello_api_ack',
         {
             's21_controller_hello_api': message['s21_controller_hello_api'],
             's21_controller_hello_api_ack': 'Client Hello ACK Message',
             's21_tx_req_api_session_id': session['s21_tx_req_api_session_id']
         },
         namespace='/controller',
         callback=s21_controller_hello_api_ack)


@socketio.on('connect', namespace='/controller')
def connect():
    if not s21_auth_blueprint_.session.authorized or s21_auth_blueprint_.token == None:
        disconnect(namespace='/controller')
        return

    # Create this thread for receiving MQTT messages.
    global s21_mqtt_client_thread_
    with s21_mqtt_client_thread_lock_:
        if s21_mqtt_client_thread_ is None:
            s21_mqtt_client_thread_ = socketio.start_background_task(s21_mqtt_client_worker_fn)

    # Create S21 RX, TX Threads
    # S21 RX Thread handles (emits) messages received over the MQTT channel.
    global s21_receiver_thread_
    with s21_receiver_thread_lock_:
        if s21_receiver_thread_ is None:
            s21_receiver_thread_ = socketio.start_background_task(s21_receiver_worker_fn)

    # S21 TX Thread handles client s21_tx_req_api messages
    global s21_transmitter_thread_
    with s21_transmitter_thread_lock_:
        if s21_transmitter_thread_ is None:
            s21_transmitter_thread_ = socketio.start_background_task(s21_transmitter_worker_fn)

    emit('s21_controller_connect_response',
         {
             'data': 'Connected',
             'count': 0
         },
         namespace='/controller')


@socketio.event
def s21_controller_ping():
    emit('s21_controller_pong')

'''
