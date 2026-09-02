<template>
  <button
    type="button"
    class="flex h-20 w-full min-w-0 items-center gap-5 rounded-xl border border-default bg-transparent p-4 text-left hover:bg-elevated/50"
  >
    <div class="flex min-w-0 flex-1 items-center gap-2">
      <div class="size-8 shrink-0 overflow-clip">
        <img
          :src="icon"
          :alt="title"
          width="32"
          height="32"
          class="size-8 [image-rendering:pixelated]"
        >
      </div>
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
defineProps<{
  title: string;
  icon: string;
}>();
</script>
