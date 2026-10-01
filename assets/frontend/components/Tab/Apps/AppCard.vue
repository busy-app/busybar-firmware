<template>
  <button
    type="button"
    class="flex h-20 w-full min-w-0 items-center gap-5 rounded-xl border border-default bg-transparent p-4 text-left hover:bg-elevated/50"
  >
    <div class="flex min-w-0 flex-1 items-center gap-2">
      <img
        :src="icon && !broken ? icon : unknownAppIcon"
        alt=""
        width="32"
        height="32"
        class="size-8 shrink-0 [image-rendering:pixelated]"
        @error="broken = true"
      >

      <div class="truncate text-xl font-medium">
        {{ title }}
      </div>
    </div>

    <UIcon
      name="i-bi-chevron-right"
      class="size-6 shrink-0 text-muted"
    />
  </button>
</template>

<script setup lang="ts">
import unknownAppIcon from '@/assets/images/unknown-app-icon.png';

const props = defineProps<{
  title: string;
  icon?: string;
}>();

const broken = ref(false);

watch(() => props.icon, () => {
  broken.value = false;
});
</script>
