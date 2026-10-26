<template>
  <div
    data-id="app-info"
    class="flex flex-col gap-4"
  >
    <div class="flex items-center gap-2">
      <img
        :src="icon && !broken ? icon : unknownAppIcon"
        alt=""
        width="48"
        height="48"
        class="size-12 shrink-0 rounded [image-rendering:pixelated]"
        @error="broken = true"
      >

      <div class="flex min-w-0 flex-1 flex-col justify-center">
        <div
          data-id="app-info-name"
          class="truncate text-xl font-medium text-highlighted"
        >
          {{ app.name }}
        </div>

        <div
          data-id="app-info-meta"
          class="truncate text-sm text-toned"
        >
          {{ meta }}
        </div>
      </div>
    </div>

    <div
      v-if="app.description"
      data-id="app-info-description"
      class="text-toned"
    >
      {{ app.description }}
    </div>
  </div>
</template>

<script setup lang="ts">
import type { AppInfo } from '@busy-app/busy-lib';
import unknownAppIcon from '@/assets/images/unknown-app-icon.png';

const props = defineProps<{
  app: AppInfo;
  installed?: AppInfo;
  icon?: string;
}>();

const broken = ref(false);

const meta = computed(() => {
  const version = props.installed && props.installed.version !== props.app.version
    ? `v${props.installed.version} \u2192 v${props.app.version}`
    : `v${props.app.version}`;

  return `${props.app.author || 'Unknown'} \u2022 ${version}`;
});

watch(() => props.icon, () => {
  broken.value = false;
});
</script>
