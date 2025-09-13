<template>
  <UCard
    :ui="{
      root: 'ring-1 ring-glass rounded-3xl bg-elevated/50 divide-none',
      header: 'p-4 sm:p-6',
      body: 'p-4 sm:p-6 pt-0 sm:pt-0',
      footer: 'p-4 sm:p-6'
    }"
  >
    <template #header>
      <div class="w-full flex justify-between items-center">
        <div class="flex items-center gap-4">
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
            <slot name="description" />
          </div>
        </div>

        <div class="flex items-center gap-4">
          <slot name="actions" />
        </div>
      </div>
    </template>

    <div
      v-if="$slots.default"
      class="flex flex-col gap-1 rounded-group"
    >
      <div
        v-for="(child, i) in $slots.default()"
        :key="child.key ?? i"
        class="bg-elevated/75 p-4 rounded-xl"
      >
        <component :is="child" />
      </div>
    </div>
  </UCard>
</template>

<script setup lang="ts">
defineProps({
  title: String,
  icon: String
});
</script>
