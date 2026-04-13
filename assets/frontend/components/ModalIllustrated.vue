<template>
  <UModal
    :content="{
      // this is the only way to pass dynamic data-id to UModal root element
      // and content is not even typed properly to include attrs
      'data-id': props.dataId
    } as unknown as undefined
    "
    :title="props.title"
    :description="props.description"
    :ui="{
      content: `${props.wide ? 'max-w-[640px]' : ''}`,
      description: 'hidden',
      header: 'hidden',
      body: 'p-0 sm:p-0 overflow-visible',
      close: showCloseButton ? 'flex top-6 end-5' :'hidden'
    }"
  >
    <template #description>
      <div :data-id="`${props.dataId}-description`">{{ props.description }}</div>
    </template>

    <template #body>
      <div
        class="flex flex-col p-5 gap-6"
      >
        <img :src="colorMode.value === 'dark' ? props.images?.dark : props.images?.light">

        <div class="flex flex-col gap-5">
          <div class="flex flex-col gap-2">
            <div
              :data-id="`${props.dataId}-title`"
              class="text-xl font-medium"
            >
              {{ props.title }}
            </div>
            <div
              v-if="props.description"
              :data-id="`${props.dataId}-description`"
            >
              {{ props.description }}
            </div>
          </div>

          <slot />
        </div>

        <template v-if="$slots.actions">
          <slot name="actions" />
        </template>
        <div
          v-else-if="!props.noActions"
          class="flex justify-end gap-2"
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
  images?: {
    light: string;
    dark: string;
  };
  primaryActionProps?: ButtonProps;
  secondaryActionProps?: ButtonProps;
  showCloseButton?: boolean;
  wide?: boolean;
  noActions?: boolean;
}>();

const colorMode = useColorMode();
</script>
