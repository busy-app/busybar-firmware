<template>
  <div
    data-id="page-login"
    class="w-full min-h-[calc(100vh-2rem)] flex flex-col items-center justify-center gap-6"
  >
    <template v-if="deviceStore.httpAPIAccess?.mode === 'disabled'">
        <LockedLayoutApiDisabled />
      </template>
      <template v-else>
        <LockedLayoutLoginForm />
      </template>
  </div>
</template>

<script setup lang="ts">
useHead({
  title: '🔒 BUSY Bar Virtual LAN',
  meta: [
    {
      name: 'description',
      content: '[Locked] Control your BUSY Bar in the browser'
    }
  ]
});

definePageMeta({
  layout: 'locked'
});

const deviceStore = useDeviceStore();
</script>
