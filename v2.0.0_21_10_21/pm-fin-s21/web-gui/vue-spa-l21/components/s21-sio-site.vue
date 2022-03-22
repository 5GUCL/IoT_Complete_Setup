<template
  v-bind:channel="sioChannel"
  v-bind:ncap="ncapId"
>
  <div>
    <v-data-table
      :headers="headers"
      :items="iot_transactions"
      :footer-props="{
        showFirstLastPage: true,
        firstIcon: 'mdi-arrow-collapse-left',
        lastIcon: 'mdi-arrow-collapse-right',
        prevIcon: 'mdi-minus',
        nextIcon: 'mdi-plus'
      }"
    >
      <template #top>
        <v-toolbar flat>
          <v-toolbar-title>FIELD DATA</v-toolbar-title>
          <v-divider
            class="mx-4"
            inset
            vertical
          />
          <div class="text-center">
            <v-btn
              class="mx-2"
              fab
              dark
              small
              color="deep-orange lighten-1"
              @click="dialogDelete = true"
            >
              <v-icon>
                mdi-delete
              </v-icon>
            </v-btn>
          </div>
          <v-dialog v-model="dialogDelete" max-width="500px">
            <v-card>
              <v-card-title class="text-h5">
                Are you sure you want to delete the data?
              </v-card-title>
              <v-card-actions>
                <v-spacer />
                <v-btn color="green darken-1" text @click="closeDelete">
                  Cancel
                </v-btn>
                <v-btn color="warning darken-1" text @click="clearData">
                  OK
                </v-btn>
                <v-spacer />
              </v-card-actions>
            </v-card>
          </v-dialog>
          <v-spacer />
          <v-toolbar-title> {{ ncap }} </v-toolbar-title>
          <v-divider
            class="mx-6"
            inset
            vertical
          />
          <div class="text-center">
            <v-btn
              fab
              dark
              small
              color="grey lighten-4"
              class="disable-events"
            >
              <v-icon
                :color="sio_conn_status_ind"
                size="32"
                class="mx-auto"
              >
                mdi-wifi-strength-4
              </v-icon>
            </v-btn>
          </div>
        </v-toolbar>
        <v-divider />
        <v-card
          flat
        >
          <v-card-title class="grey--text font-weight-light">
            {{ description }}
          </v-card-title>
        </v-card>
        <v-progress-linear
          buffer-value="0"
          stream
          :color="sio_conn_status_ind"
        />
      </template>
      <template #[getQuantitySlotControls]="{ item }">
        <v-chip
          :color="getQuantityColor(item.quantity)"
          dark
        >
          {{ item.quantity }}
        </v-chip>
      </template>
      <template #[getTxnSlotControls]="{ item }">
        <v-chip
          dark
        >
          <v-icon
            left
            :color="getTxnContextColor(item)"
            class="pa-2"
          >
            mdi-contactless-payment
          </v-icon>
          {{ item.transaction }}
        </v-chip>
      </template>
    </v-data-table>
  </div>
</template>

<script>
export default {
  props: {
    channel: {
      type: String,
      required: true,
      default: ''
    },
    ncap: {
      type: String,
      required: true,
      default: ''
    },
    description: {
      type: String,
      required: true,
      default: ''
    }
  },
  data: () => ({
    // nuxt-socket-io opts:
    sioName: 's21-sio', // Use socket "s21-sio"
    sioChannel: '',
    ncapId: '',
    dialogDelete: false,
    sioConnStatus: {},
    sioS21Conn: false,
    sio_conn_status_ind: 'red lighten-1',
    headers: [
      {
        text: '#',
        value: 'sl_num',
        align: 'start',
        sortable: false

      },
      {
        text: 'Event',
        value: 'event',
        sortable: false
      },
      {
        text: 'IoT Device',
        value: 'device',
        sortable: false
      },
      {
        text: 'Net Quantity',
        value: 'quantity',
        sortable: false
      },
      {
        text: 'Time',
        value: 'time',
        sortable: false
      },
      {
        text: 'Transaction',
        value: 'transaction',
        sortable: false,
        color: 'success'
      }
    ],
    iot_transactions: [],
    txn_counter: 1
  }),
  computed: {
    getQuantitySlotControls () {
      return 'item.quantity'
    },
    getTxnSlotControls () {
      return 'item.transaction'
    }
  },

  watch: {
    sioConnStatus (status) {
      // https://nuxt-socket-io.netlify.app/socketStatus
      /*
        const clientEvts = [
          'connect_error',
          'connect_timeout',
          'reconnect',
          'reconnect_attempt',
          'reconnecting',
          'reconnect_error',
          'reconnect_failed',
          'ping',
          'pong'
        ]
      */
      if (status === 'ping' || status === 'pong' || status === 'connect' || status === 'connected') {
        this.sio_conn_status_ind = 'success'
      } else {
        this.sio_conn_status_ind = 'red lighten-1'
      }
    },
    dialogDelete (val) {
      val || this.closeDelete()
    }
  },

  mounted () {
    this.socket = this.$nuxtSocket(
      {
        // nuxt-socket-io opts:
        name: this.sioName,
        channel: '/' + this.channel,
        namespaceCfg: { // overrides the namespace config of "examples" above
          emitters: [],
          listeners: [],
          emitBacks: []
        },
        statusProp: 'sioConnStatus',
        // socket.io-client opts:
        reconnection: true,
        teardown: true
      }
    )
    /* Listen for events: */
    this.socket
      .on('connect', (msg, cb) => {
        /* Handle event */
        this.sio_conn_status_ind = 'success'
        this.sioS21Conn = true
        // Handle callback.
        if (cb) {
          cb()
        }

        // Join the rooms that are interested.
      })
    this.socket
      .on('disconnect', () => {
        /* Handle event */
        this.sio_conn_status_ind = 'red lighten-1'
        this.sioS21Conn = false
      })
    this.socket
      .on('s21_rx_event', (msg, cb) => {
        let ctxColor

        if (msg.s21_txn_context.status_code === 200) {
          ctxColor = 'green accent-4'
        } else {
          ctxColor = 'deep-orange accent-3'
        }

        const msg_ = {
          sl_num: this.txn_counter++,
          event: msg.s21_rx_event,
          device: msg.s21_rx_event_data.ncap_id,
          quantity: msg.s21_rx_event_data.net_quantity,
          time: msg.s21_rx_event_data.time_instance,
          transaction: msg.s21_txn_context.status_str,
          txn_context: {
            color: ctxColor,
            server: msg.s21_txn_context
          }
        }

        this.iot_transactions.unshift(msg_)

        if (cb) {
          cb()
        }
      })
  },

  created () {
  },

  methods: {
    clearData () {
      this.iot_transactions = []
      this.txn_counter = 1
      this.dialogDelete = false
    },
    closeDelete () {
      this.dialogDelete = false
    },
    getQuantityColor (quantity) {
      if (quantity <= 1000) {
        return 'red accent-1'
      } else if (quantity > 1000 && quantity <= 2000) {
        return 'teal lighten-2'
      } else if (quantity > 2000 && quantity <= 3000) {
        return 'green darken-1'
      } else if (quantity > 3000 && quantity <= 4000) {
        return 'orange darken-2'
      } else {
        return 'brown darken-1'
      }
    },
    getTxnContextColor (item) {
      return item.txn_context.color
    }
  }
}
</script>

<style scoped>
  .disable-events {
    pointer-events: none
  }
</style>
