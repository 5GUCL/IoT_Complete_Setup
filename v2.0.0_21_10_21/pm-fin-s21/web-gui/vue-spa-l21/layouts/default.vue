<template>
  <v-app light>
    <v-navigation-drawer
      v-model="drawer"
      :mini-variant="miniVariant"
      :clipped="clipped"
      fixed
      app
    >
      <v-list>
        <v-list-item
          v-for="(item, i) in nav_sidebar_items"
          :key="i"
          exact
        >
          <v-tooltip bottom>
            <template #activator="{ on }">
              <v-list-item-action v-on="on">
                <v-icon
                  color="deep-orange darken-1"
                >
                  {{ item.icon }}
                </v-icon>
              </v-list-item-action>
              <v-list-item-content>
                <v-list-item-title
                  color="deep-orange darken-1"
                  v-text="item.title"
                />
              </v-list-item-content>
            </template>
            <span>{{ item.title }}</span>
          </v-tooltip>
        </v-list-item>
      </v-list>
      <v-divider />
      <v-list>
        <v-list-item
          v-for="(item, i) in items"
          :key="i"
          :to="item.to"
          router
          exact
        >
          <v-tooltip bottom>
            <template #activator="{ on }">
              <v-list-item-action v-on="on">
                <v-icon>{{ item.icon }}</v-icon>
              </v-list-item-action>
              <v-list-item-content>
                <v-list-item-title
                  v-text="item.title"
                />
              </v-list-item-content>
            </template>
            <span>{{ item.title }}</span>
          </v-tooltip>
        </v-list-item>
      </v-list>
      <v-divider />
      <v-list>
        <v-list-item
          v-for="(item, i) in core_banking_head"
          :key="i"
          exact
        >
          <v-tooltip bottom>
            <template #activator="{ on }">
              <v-list-item-action v-on="on">
                <v-icon
                  color="deep-orange darken-1"
                >
                  {{ item.icon }}
                </v-icon>
              </v-list-item-action>
              <v-list-item-content>
                <v-list-item-title
                  color="deep-orange darken-1"
                  v-text="item.title"
                />
              </v-list-item-content>
            </template>
            <span>{{ item.title }}</span>
          </v-tooltip>
        </v-list-item>
      </v-list>
      <v-divider />
      <v-list>
        <v-list-item
          v-for="(item, i) in core_banking"
          :key="i"
          :to="item.to"
          router
          exact
        >
          <v-tooltip bottom>
            <template #activator="{ on }">
              <v-list-item-action v-on="on">
                <v-icon>{{ item.icon }}</v-icon>
              </v-list-item-action>
              <v-list-item-content>
                <v-list-item-title
                  v-text="item.title"
                />
              </v-list-item-content>
            </template>
            <span>{{ item.title }}</span>
          </v-tooltip>
        </v-list-item>
      </v-list>
    </v-navigation-drawer>
    <v-navigation-drawer
      v-model="right_drawer"
      :right="right"
      :mini-variant="right_mini_variant"
      :clipped="right_clipped"
      fixed
      app
    >
      <v-list>
        <v-list-item
          v-for="(item, i) in iot_head_items"
          :key="i"
          exact
        >
          <v-tooltip bottom>
            <template #activator="{ on }">
              <v-list-item-action v-on="on">
                <v-icon
                  color="deep-orange darken-1"
                >
                  {{ item.icon }}
                </v-icon>
              </v-list-item-action>
              <v-list-item-content>
                <v-list-item-title
                  color="deep-orange darken-1"
                  v-text="item.title"
                />
              </v-list-item-content>
            </template>
            <span>{{ item.title }}</span>
          </v-tooltip>
        </v-list-item>
      </v-list>
      <v-divider />
      <v-list>
        <v-list-item
          v-for="(item, i) in iot_items"
          :key="i"
          :to="item.to"
          router
          exact
        >
          <v-tooltip bottom>
            <template #activator="{ on }">
              <v-list-item-action v-on="on">
                <v-icon>{{ item.icon }}</v-icon>
              </v-list-item-action>
              <v-list-item-content>
                <v-list-item-title
                  v-text="item.title"
                />
              </v-list-item-content>
            </template>
            <span>{{ item.title }}</span>
          </v-tooltip>
        </v-list-item>
      </v-list>
    </v-navigation-drawer>
    <v-app-bar
      :clipped-left="clipped"
      :clipped-right="right_clipped"
      fixed
      app
      color="grey lighten-4"
      elevation="2"
    >
      <v-app-bar-nav-icon @click.stop="drawer = !drawer" />
      <v-btn
        icon
        @click.stop="miniVariant = !miniVariant"
      >
        <v-icon>mdi-{{ `chevron-${miniVariant ? 'right' : 'left'}` }}</v-icon>
      </v-btn>
      <v-toolbar-title v-text="title" />
      <v-spacer />
      <v-tooltip bottom>
        <template #activator="{ on }">
          <v-btn
            fab
            dark
            small
            color="green darken-1"
            :to="docs_menu.to"
            v-on="on"
          >
            <v-icon size="24">
              {{ docs_menu.icon }}
            </v-icon>
          </v-btn>
        </template>
        <span>{{ docs_menu.title }}</span>
      </v-tooltip>
      <v-tooltip bottom>
        <template #activator="{ on }">
          <v-btn
            fab
            dark
            small
            color="indigo"
            :to="user_menu.to"
            align="center"
            class="ma-5"
            v-on="on"
          >
            <v-icon
              size="24"
            >
              {{ user_menu.icon }}
            </v-icon>
          </v-btn>
        </template>
        <span>{{ user_menu.title }}</span>
      </v-tooltip>
      <v-tooltip bottom>
        <template #activator="{ on }">
          <v-btn
            fab
            dark
            small
            color="deep-orange darken-3"
            class="ml-5 mr-14"
            align="center"
            v-on="on"
            @click.stop="dialogLogout = true"
          >
            <v-icon
              size="24"
            >
              {{ logout_menu.icon }}
            </v-icon>
          </v-btn>
        </template>
        <span>{{ logout_menu.title }}</span>
      </v-tooltip>
      <v-dialog v-model="dialogLogout" max-width="500px">
        <v-card>
          <v-card-title class="text-h5">
            Do you want to logout?
          </v-card-title>
          <v-card-actions>
            <v-spacer />
            <v-btn color="green darken-1" text @click="closeLogoutDialog">
              Cancel
            </v-btn>
            <v-btn
              color="warning darken-1"
              :href="logout_menu.to"
              text
            >
              OK
            </v-btn>
            <v-spacer />
          </v-card-actions>
        </v-card>
      </v-dialog>
      <v-btn
        icon
        @click.stop="right_mini_variant = !right_mini_variant"
      >
        <v-icon>mdi-{{ `chevron-${right_mini_variant ? 'left' : 'right'}` }}</v-icon>
      </v-btn>
      <v-app-bar-nav-icon @click.stop="right_drawer = !right_drawer" />
    </v-app-bar>
    <v-main>
      <cookie-law theme="dark-lime" />
      <v-container fluid>
        <v-btn
          v-show="fab"
          v-scroll="onScroll"
          fab
          medium
          dark
          right
          bottom
          fixed
          color="pink accent-4"
          @click="toTop"
        >
          <v-icon>mdi-arrow-up</v-icon>
        </v-btn>
        <Nuxt />
      </v-container>
    </v-main>
    <v-footer
      :absolute="!fixed"
      app
    >
      <span>&copy; Vidcentum {{ new Date().getFullYear() }}. Picominer&reg; - IoT Enabled Business Solutions.</span>
    </v-footer>
  </v-app>
</template>

<script>
import CookieLaw from 'vue-cookie-law'
export default {
  components: {
    CookieLaw
  },
  data () {
    return {
      dialogLogout: false,
      fab: false,
      clipped: true,
      drawer: true,
      fixed: true,
      miniVariant: true,
      title: 'IoT Enabled Business Solutions',
      nav_sidebar_items: [
        {
          icon: 'mdi-cloud-check-outline',
          title: 'DATA SERVICES'
        }
      ],
      items: [
        {
          icon: 'mdi-chart-bubble',
          title: 'Field Data',
          to: '/'
        },
        {
          icon: 'mdi-apps',
          title: 'Transactions',
          to: '/transactions'
        }
      ],
      docs_menu:
      {
        icon: 'mdi-notebook-multiple',
        title: 'Docs',
        to: '/docs/readme'
      },
      user_menu:
      {
        icon: 'mdi-account',
        title: 'Profile',
        to: '/user/profile'
      },
      logout_menu:
      {
        icon: 'mdi-logout-variant',
        title: 'Logout',
        to: this.$config.logoutUrl
      },
      iot_head_items: [
        {
          icon: 'mdi-layers',
          title: 'IOT INFRASTRUCTURE'
        }
      ],
      iot_items: [
        {
          icon: 'mdi-devices',
          title: 'Devices',
          to: '/iot/devices/readme'
        },
        {
          icon: 'mdi-router-network',
          title: 'Gateways',
          to: '/iot/gateways/readme'
        },
        {
          icon: 'mdi-database-cog',
          title: 'Data Sources',
          to: '/iot/data-sources/readme'
        },
        {
          icon: 'mdi-server-network',
          title: 'Servers',
          to: '/iot/servers/readme'
        }
      ],
      core_banking_head: [
        {
          icon: 'mdi-bank',
          title: 'IOT FOR BANKING'
        }
      ],
      core_banking: [
        {
          icon: 'mdi-account-star',
          title: 'Client Onboarding',
          to: '/cb/client-onboarding'
        },
        {
          icon: 'mdi-dots-hexagon',
          title: 'Recommendations',
          to: '/cb/client-recommendations'
        },
        {
          icon: 'mdi-home-city',
          title: 'Collateral Assets',
          to: '/cb/collateral-assets-indicators'
        },
        {
          icon: 'mdi-track-light',
          title: 'Loan Loss Indicators',
          to: '/cb/loan-loss-indicators'
        },
        {
          icon: 'mdi-transfer',
          title: 'Tranche Loan Assets',
          to: '/cb/tranche-loan-asset-indicators'
        }
      ],
      right_clipped: true,
      right_drawer: true,
      right: true,
      right_mini_variant: true
    }
  },
  watch: {
    dialogLogout (val) {
      // https://github.com/vuetifyjs/vuetify/issues/9808
      document.activeElement.blur()
      val || this.closeLogoutDialog()
    }
  },
  mounted: () => {
  },
  methods: {
    onScroll (e) {
      if (typeof window === 'undefined') { return }
      const top = window.pageYOffset || e.target.scrollTop || 0
      this.fab = top > 20
    },
    toTop () {
      this.$vuetify.goTo(0)
    },
    closeLogoutDialog () {
      this.dialogLogout = false
    }
  }
}
</script>
