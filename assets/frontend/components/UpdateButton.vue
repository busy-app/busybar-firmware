<template>
  <UButton
    variant="subtle"
    color="neutral"
    :ui="{
      base: 'flex self-start items-center justify-center gap-2 ring-1 ring-glass rounded-full pl-3 pr-2 py-1'
    }"
    @click="tabStore.currentTab = 'firmware'"
  >
    <div class="relative size-2.5 rounded-full bg-green-500" />
    <div class="flex items-center">
      Update&nbsp;available
      <div class="h-6 w-5 flex items-center justify-end">
        <UIcon
          name="i-ri-arrow-right-s-line"
          class="size-4"
        />
      </div>
    </div>
  </UButton>
</template>

<script setup lang="ts">
const tabStore = useTabStore();
</script>
