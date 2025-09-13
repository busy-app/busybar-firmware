<template>
  <div class="sticky top-4 w-40 self-start flex flex-col gap-2">
    <div
      v-for="item in items"
      :key="item.value"
      class="grid items-center grid-cols-[24px_auto] gap-[10px] p-3 rounded-xl cursor-pointer"
      :class="modelValue === item.value ? 'bg-elevated' : ''"
      @click="modelValue = item.value"
    >
      <UIcon
        :name="item.activeIcon ? modelValue === item.value ? item.activeIcon : item.icon : item.icon"
        class="size-6"
      />
      <div>{{ item.label }}</div>
    </div>
  </div>
</template>

<script setup lang="ts">
const modelValue = ref('network');
const items = [
  {
    label: 'Network',
    icon: 'i-tabler-wifi',
    activeIcon: 'i-tabler-wifi-off',
    value: 'network'
  },
  {
    label: 'Firmware',
    icon: 'i-tabler-file-upload',
    value: 'firmware'
  },
  {
    label: 'Settings',
    icon: 'i-tabler-bluetooth',
    value: 'settings'
  }
];
</script>
