<template>
  <div
    data-id="app-info"
    class="flex flex-col gap-2"
  >
    <div
      v-if="app.description"
      data-id="app-info-description"
    >
      {{ app.description }}
    </div>

    <div class="flex flex-col gap-1">
      <div
        v-for="field in fields"
        :key="field.label"
        :data-id="`app-info-${field.label.toLowerCase()}`"
        class="flex items-center gap-2 rounded-[9px] bg-accented/25 p-4"
      >
        <div class="min-w-0 flex-1 truncate text-muted">{{ field.label }}</div>
        <div class="min-w-0 flex-1 truncate text-right text-highlighted">{{ field.value }}</div>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import type { AppInfo } from '@busy-app/busy-lib';

const props = defineProps<{
  app: AppInfo;
  installed?: AppInfo;
}>();

const fields = computed(() => [
  { label: 'Author', value: props.app.author || 'Unknown' },
  {
    label: 'Version',
    value: props.installed && props.installed.version !== props.app.version
      ? `${props.installed.version} → ${props.app.version}`
      : props.app.version
  }
]);
</script>
