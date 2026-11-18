<template>
  <div
    data-id="layout-default-tabs"
    class="flex max-w-screen gap-2 overflow-auto px-4 sm:px-0 xl:sticky xl:top-4 xl:w-40 xl:flex-col xl:self-start"
  >
    <div
      v-for="tab in options"
      :key="tab.value"
      :data-id="tab.dataId"
      class="grid cursor-pointer grid-cols-[24px_auto] items-center gap-[10px] rounded-xl p-3 whitespace-nowrap"
      :class="tabStore.currentTab === tab.value ? 'bg-accented/50 dark:bg-elevated ring-1 ring-glass' : 'text-muted hover:text-default'"
      @click="handleTabClick(tab.value)"
    >
      <UIcon
        :name="tab.activeIcon ? tabStore.currentTab === tab.value ? tab.activeIcon : tab.icon : tab.icon"
        class="size-6"
      />
      <div class="flex w-full items-center justify-between gap-4 xl:pr-1.5">
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
