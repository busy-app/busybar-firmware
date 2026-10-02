<template>
  <SectionCard
    data-id="settings-section-timezone"
    icon="i-bi-timezone"
    title="Timezone"
  >
    <template #actions>
      <USelect
        v-if="tzListStore.timezoneOptions?.length"
        v-model="timezoneStore.timezone"
        :items="tzListStore.timezoneOptions.map(tz => ({ label: `UTC${tz.offset}, ${tz.name}`, value: tz.name }))"
        variant="soft"
        size="xl"
        class="h-12 min-w-56"
        :ui="{
          base: 'text-base rounded-xl',
          label: 'text-base',
          item: 'text-base'
        }"
        @update:model-value="timezoneStore.setTimezone(timezoneStore.timezone!)"
      />
    </template>
  </SectionCard>
</template>

<script setup lang="ts">
const timezoneStore = useTimezoneStore();
const tzListStore = useTzListStore();

async function init () {
  if (!tzListStore.timezoneOptions || tzListStore.timezoneOptions.length === 0) {
    await tzListStore.fetchTimezoneOptions();
  }
  await timezoneStore.fetchTimezone();
}

onMounted(async () => {
  await init();
  window.addEventListener('device-reconnected', init);
});
onBeforeUnmount(() => window.removeEventListener('device-reconnected', init));
</script>
