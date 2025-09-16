<template>
  <SectionCard
    title="Firmware"
    :subtitle="system?.version === 'unknown' ? `${system.branch} ${system.commit_hash}` : system?.version"
    icon="i-ri-cpu-line"
  >
    <template #actions>
      <UButton
        label="Update from file"
        icon="i-ri-upload-2-line"
        variant="link"
        :ui="{
          base: 'px-2.5 py-2 rounded-full'
        }"
      />
      <UButton
        label="Update"
        icon="i-ri-download-cloud-line"
        :ui="{
          base: 'px-2.5 py-2 rounded-full'
        }"
      />
    </template>

    <div class="grid sm:grid-cols-2 gap-y-3 gap-x-1">
      <div
        v-for="[property, value] in Object.entries({
          'Version': system?.version,
          'Build date': system?.build_date,
          'Branch': system?.branch,
          'Commit hash': system?.commit_hash,
          'Uptime': system?.uptime
        })"
        :key="property"
        class="flex"
      >
        <div class="w-[120px] text-muted">{{ property }}</div>
        <div class="max-w-[140px] md:max-w-[180px] text-ellipsis overflow-hidden">{{ value }}</div>
      </div>
    </div>
  </SectionCard>
</template>

<script setup lang="ts">
const deviceStore = useDeviceStore();

const system = computed(() => deviceStore.deviceStatus?.system);

onMounted(async () => {
  await deviceStore.getApiVersion();
  await deviceStore.getDeviceStatus();
});
</script>
