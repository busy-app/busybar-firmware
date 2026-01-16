<template>
  <div
    data-id="page-index"
    class="w-full flex flex-col gap-4"
  >
    <TabNetwork v-if="tabStore.currentTab === 'network'" />
    <TabFirmware v-if="tabStore.currentTab === 'firmware'" />
    <TabSettings v-if="tabStore.currentTab === 'settings'" />

    <template v-if="tabStore.showHiddenTabs">
      <TabAnimations v-if="tabStore.currentTab === 'animations'" />
    </template>
  </div>
</template>

<script setup lang="ts">
useHead({
  title: 'BUSY Bar Virtual LAN',
  meta: [
    {
      name: 'description',
      content: 'Control your BUSY Bar in the browser'
    }
  ]
});

const tabStore = useTabStore();
</script>
