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

# import s21_config

# This class acts as as ORM and interface to meta data.
class S21TopicToNamespaceMapper:
    def __init__(self, logger):
        self.logger_ = logger

    @staticmethod
    def get_sio_namespace(topic):
        # Site level Namespaces
        # If the received topic is starting with site/* then the namespace is /site
        if topic == 'site/IoT/Order':
            return '/site'
        elif topic == 'site/monitor':
            return '/site'
        elif topic == 'site/analytics':
            return '/site'
        elif topic == 'site/api':
            return '/site'

        # Global level Namespaces
        # If the received topic is starting with global/* then the namespace is /global
        elif topic == 'global/analytics':
            return '/global'
        elif topic == 'global/monitor':
            return '/global'
        elif topic == 'global/api':
            return '/global'

        else:
            return '/global'

    @staticmethod
    def get_sio_site_emit_target(topic, site_ncap_id):
        namespace_ = S21TopicToNamespaceMapper.get_sio_namespace(topic)
        if namespace_ == '/site':
            rooms_ = {
                # Site level NCAP Rooms
                'site': site_ncap_id,
                'data': 'data', # Every site has data room.
                'monitor': 'monitor',
                'analytics': 'analytics',  # Site analytics room.
                # Site level Global rooms
                'global_status': 'global_status'
            }

            emit_target = {
                'namespace': '/'+site_ncap_id,
                'rooms': rooms_
            }
            return emit_target

        elif namespace_ == '/global':
            rooms_ = {
                # Site level Global rooms
                'global_status': 'global_status',
                'data': 'data',  # Global data room.
                'analytics': 'analytics',  # Global analytics room.
                'monitor': 'monitor' # Global resource monitoring room.
            }

            emit_target = {
                'namespace': namespace_,
                'rooms': rooms_
            }
            return emit_target

        else:
            namespace_ = '/global'
            rooms_ = {
                # Site level Global rooms
                'global_status': 'global_status'
            }
            emit_target = {
                'namespace': namespace_,
                'rooms': rooms_
            }
            return emit_target
