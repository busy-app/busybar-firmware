<template>
  <ContentList :items="firmwareContent" />
</template>

<script setup lang="ts">
const deviceStore = useDeviceStore();
const uptime = useUptime();

const firmware = computed(() => deviceStore.deviceStatus?.firmware);

const firmwareContent = computed(() => [
  [
    {
      title: 'Version',
      value: firmware.value?.version,
      loading: !firmware.value
    },
    {
      title: 'Branch',
      value: firmware.value?.branch,
      loading: !firmware.value
    },
    {
      title: 'Commit hash',
      value: firmware.value?.commit_hash,
      loading: !firmware.value
    }
  ],
  [
    {
      title: 'Build date',
      value: firmware.value?.build_date,
      loading: !firmware.value
    },
    {
      title: 'API version',
      value: deviceStore.apiVersion?.api_semver,
      loading: !deviceStore.apiVersion
    },
    {
      title: 'Uptime',
      value: uptime.value,
      loading: !uptime.value
    }
  ]
]);
</script>
