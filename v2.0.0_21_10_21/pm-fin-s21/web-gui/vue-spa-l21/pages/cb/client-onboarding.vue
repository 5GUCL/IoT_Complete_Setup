<template>
  <v-row class="ft-container-80vh ftlp01-bg">
    <v-col class="text-center">
      <h2>Placeholder Page</h2>
      <blockquote class="blockquote">
        &#8220;First, solve the problem. Then, write the code.&#8221;
      </blockquote>
    </v-col>
  </v-row>
</template>

<style scoped>
@media screen and (max-width: 991px) {
  .ftlp01-bg {
      min-height: 80vh;
  }
}

@media screen and (min-width: 992px)  {
  .ftlp01-bg {
    min-height: 80vh;
  }
}
.ft-container-80vh {
  min-height: 80vh;
  display: flex;
  margin: 0 auto;
  justify-content: center;
  align-items: center;
  text-align: center;
}
</style>
