<template>
  <div
    data-id="layout-default-tabs"
    class="max-w-screen xl:sticky xl:top-4 xl:w-40 xl:self-start flex xl:flex-col gap-2 overflow-auto px-4 sm:px-0"
  >
    <div
      v-for="tab in options"
      :key="tab.value"
      :data-id="tab.dataId"
      class="grid items-center grid-cols-[24px_auto] gap-[10px] p-3 rounded-xl cursor-pointer whitespace-nowrap"
      :class="tabStore.currentTab === tab.value ? 'bg-accented/50 dark:bg-elevated ring-1 ring-glass' : 'text-muted hover:text-default'"
      @click="handleTabClick(tab.value)"
    >
      <UIcon
        :name="tab.activeIcon ? tabStore.currentTab === tab.value ? tab.activeIcon : tab.icon : tab.icon"
        class="size-6"
      />
      <div class="w-full flex items-center justify-between gap-4 xl:pr-1.5">
        {{ tab.label }}

        <div
          v-if="firmwareStore.autoUpdate.status === 'available' && tab.value === 'firmware'"
          class="size-2 rounded-full bg-success"
        />
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
const tabStore = useTabStore();
const firmwareStore = useFirmwareStore();
const drawToolEditorStore = useDrawToolEditorStore();

const options = computed(() => {
  return tabStore.tabOptions.filter(tab => {
    if (tab.hidden) {
      return tabStore.showHiddenTabs;
    }
    return true;
  });
});

async function handleTabClick (nextTab: TabOption['value']) {
  if (tabStore.currentTab === nextTab) {
    return;
  }

  if (tabStore.currentTab !== 'draw-tool') {
    tabStore.currentTab = nextTab;
    return;
  }

  await drawToolEditorStore.requestLeaveEditor(() => {
    tabStore.currentTab = nextTab;
  });
}
</script>
