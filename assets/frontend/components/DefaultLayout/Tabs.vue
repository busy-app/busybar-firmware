<template>
  <div class="sticky top-4 w-40 self-start flex flex-col gap-2">
    <div
      v-for="tab in tabStore.tabOptions"
      :key="tab.value"
      class="grid items-center grid-cols-[24px_auto] gap-[10px] p-3 rounded-xl cursor-pointer"
      :class="tabStore.currentTab === tab.value ? 'bg-elevated' : ''"
      @click="tabStore.currentTab = tab.value"
    >
      <UIcon
        :name="tab.activeIcon ? tabStore.currentTab === tab.value ? tab.activeIcon : tab.icon : tab.icon"
        class="size-6"
      />
      <div>{{ tab.label }}</div>
    </div>
  </div>
</template>

<script setup lang="ts">
const tabStore = useTabStore();
</script>
