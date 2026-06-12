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
      body: `${props.stickyActions ? 'pt-0 sm:pt-0 sm:px-5 sm:pb-5 overflow-hidden flex flex-col min-h-0' : 'pt-0 sm:pt-0 sm:px-5 sm:pb-5 overflow-y-auto'}`,
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
      <div
        v-if="props.stickyActions"
        class="flex h-full min-h-0 flex-col gap-6"
      >
        <div class="min-h-0 flex-1 overflow-y-auto pr-1">
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
          class="flex shrink-0 justify-end gap-2"
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
      </div>

      <div
        v-else
        class="flex flex-col gap-6"
      >
        <div
          v-if="props.description"
          :data-id="`${props.dataId}-description`"
        >
          {{ props.description }}
        </div>
        <slot name="body" />

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
  stickyActions?: boolean;
  noActions?: boolean;
}>();
</script>
