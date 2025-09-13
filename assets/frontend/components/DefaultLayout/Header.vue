<template>
  <nav class="relative h-12 flex justify-between items-center">
    <div class="flex gap-4">
      <UIcon
        name="i-busy-bar-logo"
        class="w-[70px] h-[28px]"
      />
      <div class="hidden sm:flex items-center gap-2">
        <div class="flex items-center gap-1">
          <UIcon
            name="i-tabler-usb"
            class="w-[18px] h-[22px]"
          />
          Connected
        </div>
        <div class="text-muted">10.0.4.20</div>
      </div>
    </div>
    <div class="absolute left-1/2 -translate-x-1/2">BUSY Bar</div>
    <div>
      <UButton
        icon="i-tabler-login-2"
        size="lg"
        square
        color="neutral"
        variant="soft"
        class="rounded-full"
      />
    </div>
  </nav>
</template>

<script setup lang="ts">
</script>
