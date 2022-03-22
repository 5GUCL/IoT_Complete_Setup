<template>
  <div class="mt-5">
    <v-row
      justify="center"
    >
      <v-col
        justify="center"
        cols="1"
      />
      <v-col
        justify="center"
        cols="10"
      >
        <article flat>
          <nuxt-content :key="refreshPageKey" :document="doc" />
        </article>
      </v-col>
      <v-col>
        <v-btn
          fab
          dark
          small
          color="blue"
          cols="1"
          align="left"
          @click="dialogPageRefresh = true"
        >
          <v-icon>
            mdi-refresh
          </v-icon>
        </v-btn>
      </v-col>
    </v-row>
    <v-dialog v-model="dialogPageRefresh" max-width="500px">
      <v-card>
        <v-card-title class="text-h5">
          Refresh the page data?
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
export default {
  async asyncData ({ $content, params }) {
    const doc = await $content('iot/data-sources', params.slug).fetch()

    return { doc }
  },
  data: () => ({
    dialogPageRefresh: false,
    refreshPageKey: 0
  }),
  watch: {
    dialogPageRefresh (val) {
      val || this.closeRefesh()
    }
  },
  methods: {
    forceRerender () {
      this.refreshPageKey += 1
      this.dialogPageRefresh = false
    },
    closeRefesh () {
      this.dialogPageRefresh = false
    }
  }
}
</script>
