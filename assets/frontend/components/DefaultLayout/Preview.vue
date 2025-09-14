<template>
  <div class="w-full flex flex-col items-center">
    <ScreenStream class="py-6" />

    <div class="flex items-center justify-center gap-6">
      <UpdateButton />

      <div class="flex items-center gap-1.5">
        <div class="relative flex">
          <UIcon
            name="i-busy-battery-charging"
            class="size-6"
          />
          <UIcon
            name="i-busy-charging-lightning"
            class="absolute size-6"
          />
        </div>
        <div>95%</div>
      </div>

      <div class="flex items-center gap-1.5">
        <CircularProgress
          :model-value="3.2 / 8 * 100"
          color="#6A7282"
          track-color="#99A1AF26"
        />
        <div>3.2 / 8 GB</div>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
</script>
