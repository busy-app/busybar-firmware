<template>
  <UModal
    :title="props.title"
    :ui="{
      content: `${props.wide ? 'max-w-[640px]' : 'max-w-[360px]'} divide-none`,
      description: 'hidden',
      header: 'min-h-20',
      body: 'pt-0 sm:pt-0 overflow-y-scroll',
      close: showCloseButton ? 'flex top-6 end-5' :'hidden'
    }"
  >
    <template #title>
      <div class="text-xl">{{ props.title }}</div>
    </template>

    <template #description>
      {{ props.description }}
    </template>

    <template #body>
      <div class="flex flex-col gap-6">
        <div v-if="props.description">{{ props.description }}</div>
        <slot name="body" />
      </div>

      <template v-if="$slots.actions">
        <slot name="actions" />
      </template>
      <div
        v-else-if="!props.noActions"
        class="flex justify-end gap-2 mt-8"
      >
        <UButton
          v-bind="props.secondaryActionProps"
          variant="ghost"
          color="neutral"
        />
        <UButton
          v-bind="props.primaryActionProps"
        />
      </div>
    </template>
  </UModal>
</template>

<script setup lang="ts">
import type { ButtonProps } from '@nuxt/ui';

const props = defineProps<{
  title: string;
  description?: string;
  primaryActionProps?: ButtonProps;
  secondaryActionProps?: ButtonProps;
  showCloseButton?: boolean;
  wide?: boolean;
  noActions?: boolean;
}>();
</script>
