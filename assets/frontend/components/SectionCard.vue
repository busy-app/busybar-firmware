<template>
  <UCard
    :ui="{
      root: 'ring-1 ring-glass rounded-3xl bg-elevated/50 divide-none',
      header: 'p-4 sm:p-6',
      body: 'p-4 sm:p-6 pt-0 sm:pt-0',
      footer: 'p-4 sm:p-6'
    }"
  >
    <template
      v-if="headerExists"
      #header
    >
      <div class="w-full flex flex-col sm:flex-row justify-between items-start sm:items-center gap-4">
        <div class="flex items-center gap-4">
          <slot name="leading-actions" />

          <div
            v-if="icon"
            class="size-14 rounded-full bg-elevated p-[14px]"
          >
            <UIcon
              :name="icon"
              class="size-7"
            />
          </div>

          <div>
            <div class="text-xl">{{ title }}</div>
            <div
              v-if="subtitle && !$slots.subtitle"
              class="text-muted"
            >
              {{ subtitle }}
            </div>
            <slot name="subtitle" />
          </div>
        </div>

        <div class="w-full sm:w-fit flex flex-col sm:flex-row items-stretch sm:items-end gap-2">
          <slot name="actions" />
        </div>
      </div>
    </template>

    <template
      v-if="bodyExists"
      #default
    >
      <div
        v-if="!isEmptySlot('default')"
        class="flex flex-col gap-1 rounded-group"
        :class="headerExists ? '' : 'mt-4 sm:mt-6'"
      >
        <div
          v-for="(child, i) in $slots.default!()"
          :key="child.key ?? i"
          class="bg-elevated/75 p-4 rounded-xl"
        >
          <component :is="child" />
        </div>
      </div>

      <template v-if="$slots['raw-body']">
        <slot name="raw-body" />
      </template>
    </template>
  </UCard>
</template>

<script setup lang="ts">
const props = defineProps<{
  title?: string;
  subtitle?: string;
  icon?: string;
}>();

const slots = useSlots();

const headerExists = !!(props.title || props.subtitle || props.icon || isEmptySlot('subtitle') || isEmptySlot('leading-actions') || isEmptySlot('actions'));
const bodyExists = computed(() => isEmptySlot('default') === false || isEmptySlot('raw-body') === false);

function isEmptySlot (slotName: string): boolean {
  if (!slots[slotName]) {
    return true;
  }
  const slot = slots[slotName]?.();
  return !(slot && slot.length && slot.some(vnode => vnode.children && vnode.children !== 'v-if' && vnode.children.length));
}
</script>
