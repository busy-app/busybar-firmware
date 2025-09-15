<template>
  <UModal
    :title="props.title"
    :ui="{
      content: 'max-w-[360px] divide-none',
      description: 'hidden',
      header: 'pb-0 sm:pt-0',
      body: 'pt-1 sm:pt-1',
      close: showCloseButton ? 'flex top-5 end-5' :'hidden'
    }"
  >
    <template #title>
      <div class="text-xl">{{ props.title }}</div>
    </template>

    <template #description>
      {{ props.description }}
    </template>

    <template #body>
      <template v-if="!$slots.body">{{ props.description }}</template>
      <slot name="body" />

      <template v-if="$slots.actions">
        <slot name="actions" />
      </template>
      <div
        v-else
        class="flex justify-end gap-2 mt-8"
      >
        <UButton
          v-bind="props.secondaryActionProps"
          variant="ghost"
          class="px-3 py-2.5 rounded-full"
        />
        <UButton
          v-bind="props.primaryActionProps"
          class="px-3 py-2.5 rounded-full"
        />
      </div>
    </template>
  </UModal>
</template>

<script setup lang="ts">
import type { ButtonProps } from '@nuxt/ui';

const props = defineProps<{
  title: string;
  description: string;
  primaryActionProps?: ButtonProps;
  secondaryActionProps?: ButtonProps;
  showCloseButton?: boolean;
}>();
</script>
