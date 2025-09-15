<template>
  <UButton
    :variant="enabled ? 'solid' : 'soft'"
    color="neutral"
    class="p-3 rounded-xl"
    :loading="props.loading"
  >
    <div class="w-full flex justify-between items-center">
      <div class="flex items-center gap-2">
        <UIcon
          v-if="!props.loading"
          :name="props.icon"
          class="h-5 w-5"
        />

        <div class="flex flex-col items-start">
          <span
            class="overflow-x-hidden text-nowrap text-ellipsis text-label"
            :style="`max-width: calc(${props.maxWidth || '180px'} - 74px);`"
          >{{ props.title }}</span>
          <span class="text-neutral-500">{{ props.subtitle }}</span>
        </div>
      </div>

      <UIcon
        name="i-ri-arrow-right-s-line"
        class="h-4 w-4"
      />
    </div>
  </UButton>
</template>

<script setup lang="ts">
const props = defineProps<{
  enabled: boolean;
  loading?: boolean;
  icon: string;
  title: string;
  subtitle: string;
  maxWidth?: string;
}>();
</script>
