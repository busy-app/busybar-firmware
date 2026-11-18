<template>
  <UDropdownMenu
    v-model:open="open"
    :items="props.items"
    :content="{
      align: 'end',
      side: 'bottom',
      sideOffset: 8
    }"
    :ui="{
      content: 'w-66 bg-elevated ring-accented/50',
      group: 'border-accented/50',
      item: 'items-start data-[state=open]:before:bg-accented/50 data-highlighted:before:bg-accented/50',
      itemLeadingIcon: 'mt-0.5',
      itemLabel: 'whitespace-normal text-start',
      itemLabelExternalIcon: 'hidden'
    }"
  >
    <slot />

    <template #item-label="{ item }">
      <span class="flex flex-col gap-0.5">
        <span>{{ item.label }}</span>
        <span
          v-if="item.description"
          class="text-xs text-muted"
        >
          {{ item.description }}
        </span>
      </span>
    </template>
  </UDropdownMenu>
</template>

<script setup lang="ts">
import type { DropdownMenuItem } from '@nuxt/ui';

const props = defineProps<{
  items: DropdownMenuItem[];
}>();

const open = defineModel<boolean>('open', { default: false });
</script>
