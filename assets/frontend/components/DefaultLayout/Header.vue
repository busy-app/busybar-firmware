<template>
  <nav class="relative h-12 flex justify-between items-center">
    <div class="flex gap-4">
      <UIcon
        name="i-busy-bar-logo"
        class="w-[70px] h-[28px]"
      />
      <div class="hidden md:flex items-center gap-2">
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

    <div class="absolute left-1/2 -translate-x-1/2">
      <UDropdownMenu
        :items="[
          {
            label: 'Rename',
            icon: 'i-tabler-wifi'
          },
          {
            label: 'Restart',
            icon: 'i-tabler-wifi'
          }
        ]"
        :content="{
          align: 'start',
          side: 'bottom',
          sideOffset: 8
        }"
        :ui="{
          content: 'w-24'
        }"
      >
        <UButton
          label="BUSY Bar"
          size="lg"
          trailing-icon="i-tabler-chevron-down"
          color="neutral"
          variant="ghost"
          class="text-xl"
        />
      </UDropdownMenu>
    </div>

    <div class="flex gap-4 items-center">
      <UDropdownMenu
        :items="[
          {
            label: 'Lock',
            icon: 'i-tabler-wifi'
          },
          {
            label: 'Log in to BUSY Cloud',
            icon: 'i-tabler-login-2'
          }
        ]"
        :content="{
          align: 'end',
          side: 'bottom',
          sideOffset: 8
        }"
        :ui="{
          content: 'w-48'
        }"
      >
        <UButton
          icon="i-tabler-login-2"
          size="lg"
          square
          color="neutral"
          variant="soft"
          class="rounded-full"
        />
      </UDropdownMenu>

      <ToggleTheme />
    </div>
  </nav>
</template>

<script setup lang="ts">
</script>
