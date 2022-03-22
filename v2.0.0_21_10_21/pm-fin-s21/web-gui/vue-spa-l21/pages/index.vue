<template>
  <div>
    <v-row
      fluid
      justyfy="center"
    >
      <v-breadcrumbs
        :items="bc_items"
        large
      />
    </v-row>
    <v-row
      fluid
      justify="center"
    >
      <v-col cols="3">
        <v-treeview
          :active.sync="active"
          :items="sites"
          :load-children="fetchSites"
          :open.sync="open"
          item-key="id"
          activatable
          color="indigo"
          open-on-click
          transition
          item-disabled="locked"
          shaped
          class="mt-0"
        >
          <template #prepend="{ item }">
            <v-icon
              v-if="!item.children"
              medium
            >
              mdi-router-network
            </v-icon>
          </template>
        </v-treeview>
      </v-col>
      <v-divider
        vertical
        class="mt-4"
      />
      <v-col
        justify="center"
        cols="9"
      >
        <v-scroll-y-transition mode="out-in">
          <div
            v-if="!selected"
            class="text-h6 grey--text font-weight-light mt-4"
            style="align-self: center;"
          >
            SELECT A SITE
          </div>
          <s21-sio-site
            v-if="selected"
            :key="selected.id"
            :channel="selected.ncap_id"
            :ncap="selected.ncap_id"
            :description="selected.description"
          />
        </v-scroll-y-transition>
      </v-col>
    </v-row>
    <v-dialog v-model="dialogConnRefresh" max-width="500px">
      <v-card>
        <v-card-title class="text-h5">
          Refresh the data and the connection?
        </v-card-title>
        <v-card-actions>
          <v-spacer />
          <v-btn color="green darken-1" text @click="closeRefesh">
            Cancel
          </v-btn>
          <v-btn color="warning darken-1" text @click="forceRerender">
            OK
          </v-btn>
          <v-spacer />
        </v-card-actions>
      </v-card>
    </v-dialog>
  </div>
</template>

<script>

const pause = ms => new Promise(resolve => setTimeout(resolve, ms))

export default {
  data: () => ({
    active: [],
    open: [],
    ncaps: [],

    myChannel: 'NCAP-FIN-0001',
    myNcapId: 'NCAP-FIN-0001',
    scrollToTop: true,
    dialogConnRefresh: false,
    componentKey: 0,
    bc_items: [
      {
        text: 'Portal',
        disabled: true,
        href: '#'
      },
      {
        text: 'IoT Data',
        disabled: true,
        href: '/'
      }
    ]
  }),
  computed: {
    sites () {
      return [
        {
          name: 'SITES',
          children: this.ncaps
        }
      ]
    },
    selected () {
      if (!this.active.length) {
        return undefined
      }
      const id = this.active[0]
      const sel_ = this.ncaps.find(ncap => ncap.id === id)
      return sel_
    }
  },
  watch: {
    dialogConnRefresh (val) {
      val || this.closeRefesh()
    },
    watch: {
      selected: 'reloadSiteData'
    }
  },
  methods: {
    forceRerender () {
      this.componentKey += 1
      this.dialogConnRefresh = false
    },
    closeRefesh () {
      this.dialogConnRefresh = false
    },
    async fetchSites (item) {
      // Remove in 6 months and say
      // you've made optimizations! :)
      await pause(1000)

      return fetch(this.$config.baseUrl + '/sites')
        .then(res => res.json())
        .then(json => (item.children.push(...json)))
        .catch(err => console.warn(err))
    },
    reloadSiteData () {
      this.myChannel = this.selected.ncap_id
      this.myNcapId = this.selected.ncap_id
    }
  }
}
</script>
