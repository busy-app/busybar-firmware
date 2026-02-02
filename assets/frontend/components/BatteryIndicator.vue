<template>
  <div class="relative">
    <UIcon
      v-bind="$attrs"
      :name="iconName"
    />
    <UIcon
      v-if="state === 'charging'"
      v-bind="$attrs"
      name="i-busy-charging-lightning"
      class="absolute top-0 left-0"
    />
  </div>
</template>

<script setup lang="ts">
import type { StatusPower } from '@busy-app/busy-lib';

const props = defineProps<{
  charge: StatusPower['battery_charge'];
  state: StatusPower['state'] | string; // to allow manually triggered unknown states
}>();

const iconName = computed(() => {
  const charge = props.charge ?? 0;
  const state = props.state ?? '';

  if (state === 'charging') {
    return 'i-busy-battery-charging';
  }
  if (state === 'charged' || charge >= 95) {
    return 'i-busy-battery-full';
  }
  if (charge >= 66) {
    return 'i-busy-battery-discharging-3';
  }
  if (charge >= 33) {
    return 'i-busy-battery-discharging-2';
  }
  if (charge < 33 && charge >= 0) {
    return 'i-busy-battery-discharging-1';
  }

  return 'i-busy-battery-error';
});
</script>
