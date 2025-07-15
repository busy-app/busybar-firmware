<template>
  <UButton
    :active="isActive"
    color="neutral"
    variant="ghost"
    active-variant="soft"
    size="lg"
    class="navigation-button w-full py-4 rounded-2xl ring-0 shadow-none"
    :to="props.to"
  >
    <div class="flex gap-3 items-center">
      <template v-if="props.icon">
        <UIcon
          :name="props.icon"
          class="w-6 h-6"
          :class="isActive ? '' : 'text-neutral-500'"
        />
      </template>
      <slot
        v-else
        name="icon"
      />

      <template v-if="props.label">
        <div
          class="flex items-center"
          :class="isActive ? '' : 'text-neutral-500'"
        >
          {{ props.label }}
        </div>
      </template>
      <slot
        v-else
        name="label"
      />
    </div>
  </UButton>
</template>

<script setup lang="ts">
const props = defineProps<{
  to: string;
  icon?: string;
  label?: string;
}>();

const route = useRoute();
const isActive = computed(() => route.path === props.to);
</script>

<style lang="css" scoped>
.light .navigation-button {
  --background-color-elevated: var(--color-white);
  --background-color-accented: var(--ui-color-neutral-50);
}

.dark .navigation-button {
  --background-color-elevated: var(--ui-color-neutral-900);
  --background-color-accented: var(--ui-color-neutral-800);
}
</style>
