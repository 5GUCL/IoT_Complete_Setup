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

# Application Configuration
Config = {
        'mqtt_broker' :
                {
                        'mqtt_server_name_': 'wss.emq.farout.in',
                        'mqtt_server_port_': 443,
                        'mqtt_client_id_' : 'PAHO_TEST_CLIENT_01',
                        'mqtt_ws_path_':  '/',
                        'mqtt_topics_': 'site/IoT/Order',
                        'ca_cert_': 's21_security/letsencrypt-cacert.pem'
                },
        'auth':
                {
                        'username_': '<IDP User>',
                        'password_': '<IDP User Password>',
                        'grant_type_': 'password',
                        'client_id_': 'edge-pm-fin-l21-client',
                        'client_secret_': '3288ac10-48c3-42be-b2d7-8101c7d7ceb4',
                        'base_url_': "https://idp.farout.in",
                        'token_url_': 'https://idp.farout.in/auth/realms/devices/protocol/openid-connect/token',
                        'authorization_url_': 'https://idp.farout.in/auth/realms/devices/protocol/openid-connect/auth',
                        'end_session_endpoint': 'https://idp.farout.in/auth/realms/devices/protocol/openid-connect/logout',
                        'userinfo_endpoint': 'https://idp.farout.in/auth/realms/devices/protocol/openid-connect/userinfo'
                },
        'api_server':
                {
                        'base_url': 'http://localhost:5000',
                        'device_token_endpoint': '/pub/auth/device-token',
                }
    }
