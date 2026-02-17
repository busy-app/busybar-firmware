<template>
  <UModal
    :content="{
      // this is the only way to pass dynamic data-id to UModal root element
      // and content is not even typed properly to include attrs
      'data-id': props.dataId
    } as unknown as undefined
    "
    :title="props.title"
    :ui="{
      content: `${props.wide ? 'max-w-[640px]' : ''}`,
      description: 'hidden',
      header: 'min-h-20',
      title: 'text-xl flex items-center gap-2',
      body: 'pt-0 sm:pt-0 sm:px-5 sm:pb-5 overflow-y-auto',
      close: showCloseButton ? 'flex top-6 end-5' :'hidden'
    }"
  >
    <template #title>
      <slot name="icon" />
      <div
        :data-id="`${props.dataId}-title`"
        class="text-xl"
      >
        {{ props.title }}
      </div>
    </template>

    <template #description>
      <div :data-id="`${props.dataId}-description`">{{ props.description }}</div>
    </template>

    <template #body>
      <div class="flex flex-col gap-6">
        <div
          v-if="props.description"
          :data-id="`${props.dataId}-description`"
        >
          {{ props.description }}
        </div>
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
          v-if="props.secondaryActionProps"
          v-bind="props.secondaryActionProps"
          :data-id="`${props.dataId}-secondary-action`"
          variant="ghost"
          size="lg"
          :color="props.secondaryActionProps.color || 'neutral'"
          class="min-w-20 justify-center"
        />
        <UButton
          v-if="props.primaryActionProps"
          v-bind="props.primaryActionProps"
          :data-id="`${props.dataId}-primary-action`"
          :color="props.primaryActionProps.color || 'neutral'"
          size="lg"
          class="min-w-20 justify-center"
        />
      </div>
    </template>
  </UModal>
</template>

<script setup lang="ts">
import type { ButtonProps } from '@nuxt/ui';

const props = defineProps<{
  dataId: string;
  title: string;
  description?: string;
  primaryActionProps?: ButtonProps;
  secondaryActionProps?: ButtonProps;
  showCloseButton?: boolean;
  wide?: boolean;
  noActions?: boolean;
}>();
</script>
