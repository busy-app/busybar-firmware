<script setup lang="ts">
const colorMode = useColorMode();
const isDark = computed({
  get () {
    return colorMode.value === 'dark';
  },
  set () {
    colorMode.preference = colorMode.value === 'dark' ? 'light' : 'dark';
  }
});
const toggleModel = ref(isDark.value);
</script>

<template>
  <USwitch
    checked-icon="i-tabler-moon-filled"
    unchecked-icon="i-tabler-sun-filled"
    :model-value="toggleModel"
    :ui="{
      base: 'data-[state=unchecked]:bg-neutral-200'
    }"
    @update:model-value="toggleModel = $event; isDark = $event"
  />
</template>
