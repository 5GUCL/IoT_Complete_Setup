import colors from 'vuetify/es5/util/colors'

export default {
  // Disable server-side rendering: https://go.nuxtjs.dev/ssr-mode
  ssr: false,

  // Target: https://go.nuxtjs.dev/config-target
  target: 'static',

  // Global page headers: https://go.nuxtjs.dev/config-head
  head: {
    titleTemplate: '%s',
    title: 'Picominer S21 Data Portal',
    htmlAttrs: {
      lang: 'en'
    },
    meta: [
      { charset: 'utf-8' },
      { name: 'viewport', content: 'width=device-width, initial-scale=1' },
      { hid: 'description', name: 'description', content: '' },
      { name: 'format-detection', content: 'telephone=no' }
    ],
    link: [
      { rel: 'icon', type: 'image/x-icon', href: 'static/favicon.ico' }
    ]
  },

  // Global CSS: https://go.nuxtjs.dev/config-css
  css: [
    '~/assets/page-transition.css'
  ],

  // Plugins to run before rendering page: https://go.nuxtjs.dev/config-plugins
  plugins: [
  ],

  // Auto import components: https://go.nuxtjs.dev/config-components
  components: true,

  // Modules for dev and build (recommended): https://go.nuxtjs.dev/config-modules
  buildModules: [
    // https://go.nuxtjs.dev/eslint
    '@nuxtjs/eslint-module',
    // https://go.nuxtjs.dev/vuetify
    '@nuxtjs/vuetify',

    // https://pwa.nuxtjs.org/setup
    '@nuxtjs/pwa',
  ],

  // Modules: https://go.nuxtjs.dev/config-modules
  modules: [
    // https://go.nuxtjs.dev/axios
    '@nuxtjs/axios',

    // https://www.npmjs.com/package/nuxt-socket-io
    'nuxt-socket-io',

    // https://content.nuxtjs.org/installation
    '@nuxt/content',
  ],
  content: {
    markdown: {
      prism: {
        theme: 'prism-themes/themes/prism-cb.css'
      }
    }
  },

  // Socket-I/O configiration is imported from socket-io.config.
  io: {
    warnings: false,
    sockets: [
        {
            name: 's21-sio',
            url: 'ws://localhost:5000',
            default: true,
            transports: [ "websocket" ],
            vuex: { 
                mutations: [],
                actions: [],
                emitBacks: [] 
            }, 
            namespaces: {
              '/controller': {
                  emitters: [],
                  listeners: [],
                  emitBacks: []
              },
              '/supervisor': {
                  emitters: [],
                  listeners: [],
                  emitBacks: []
              },
              '/site': {
                  emitters: [],
                  listeners: [],
                  emitBacks: []
              },
              '/global': {
                  emitters: [],
                  listeners: [],
                  emitBacks: []
              }
            }
        }
    ]
  },

  // Axios module configuration: https://go.nuxtjs.dev/config-axios
  axios: {},

  // Vuetify module configuration: https://go.nuxtjs.dev/config-vuetify
  vuetify: {
    customVariables: ['~/assets/variables.scss'],
    theme: {
      light: true,
      themes: {
        dark: {
          primary: colors.blue.darken2,
          accent: colors.grey.darken3,
          secondary: colors.amber.darken3,
          info: colors.teal.lighten1,
          warning: colors.amber.base,
          error: colors.deepOrange.accent4,
          success: colors.green.accent3
        },
        light: {
          primary: '#1976D2',
          secondary: '#424242',
          accent: '#82B1FF',
          error: '#FF5252',
          info: '#2196F3',
          success: '#4CAF50',
          warning: '#FFC107',
        }
      }
    }
  },

  // Build Configuration: https://go.nuxtjs.dev/config-build
  // static folder is needed as the dist is served from flask.
  // v1.0.0 is the Gui App version
  // in <flask>/static/<create a link $version>  to <dist-$version>/static 
  // We ensure the static files are correctly mapped to the flask - static file serving.
  build: {
	  publicPath: 'static/v1.0.0/',	  
  },

  // Keep the dist file in the version. 
  generate: {
    dir: 'dist-v1.0.0'
  },

  // App specific configuration
  
  publicRuntimeConfig: {
    logoutUrl: 'http://localhost:5000/logout',
    baseUrl: 'http://localhost:5000'
  }
}
