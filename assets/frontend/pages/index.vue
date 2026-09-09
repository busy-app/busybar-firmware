<template>
  <div
    data-id="page-index"
    class="w-full flex flex-col gap-4"
  >
    <TabNetwork v-if="tabStore.currentTab === 'network'" />
    <TabFirmware v-if="tabStore.currentTab === 'firmware'" />
    <TabSettings v-if="tabStore.currentTab === 'settings'" />
    <TabDrawTool v-if="tabStore.currentTab === 'draw-tool'" />

    <template v-if="tabStore.showHiddenTabs">
      <TabApps v-if="tabStore.currentTab === 'apps'" />
      <TabFiles v-if="tabStore.currentTab === 'files'" />
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
