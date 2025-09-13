<template>
  <div class="w-full flex flex-col">
    <div class="w-full flex justify-end">
      <UButton
        variant="soft"
        color="neutral"
        label="Log in to BUSY Cloud"
        icon="i-tabler-login-2"
        class="rounded-full py-2"
        to="https://cloud.busy.app"
        target="_blank"
      />
    </div>
  </div>
</template>

<script setup lang="ts">
useHead({
  title: 'BUSY Bar Control Panel',
  meta: [
    {
      name: 'description',
      content: 'Set up and control your Busy Bar in the browser'
    }
  ]
});
</script>
