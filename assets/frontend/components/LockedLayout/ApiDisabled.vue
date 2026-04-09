<template>
  <div class="flex flex-col gap-4">
    <UIcon
      :name="colorMode.value === 'dark' ? 'i-bi-wifi-disabled-dark' : 'i-bi-wifi-disabled'"
      class="size-10"
    />

    <div>
      <div class="w-full text-2xl font-medium text-highlighted mb-2">Wi-Fi control disabled</div>
      <div class="w-full text-toned">To control your BUSY Bar via Wi-Fi, you must provide access to HTTP API.</div>
    </div>

    <div class="flex flex-col lg:flex-row gap-4 pt-11">
      <div
        v-for="item in items"
        :key="item.id"
        class="flex flex-col gap-4"
      >
        <div class="w-84 flex items-center justify-center bg-surface-container rounded-xl">
          <img
            :src="colorMode.value === 'dark' ? item.imageDark : item.image"
            class="w-[336px]"
          >
        </div>
        <div class="flex flex-col gap-3">
          <div class="w-fit flex items-center justify-center shrink-0 rounded-lg bg-accented/50 px-3 py-1 text-sm font-medium">Step {{ item.id }}</div>
          <div class="max-w-72 text-sm">
            <template v-if="item.id === 1">Connect the device to your computer using a USB cable</template>
            <template v-if="item.id === 2">
              Open Virtual LAN at
              <a
                href="http://10.0.4.20"
                target="_blank"
                class="inline-flex items-center gap-1 text-primary font-medium"
              >
                10.0.4.20
                <UIcon
                  name="i-bi-open-in-new"
                  class="size-4"
                />
              </a>
            </template>
            <template v-if="item.id === 3">Enable HTTP API access in the Virtual LAN</template>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import apiDisabled1 from '@/assets/images/api-disabled-1.png';
import apiDisabled2 from '@/assets/images/api-disabled-2.png';
import apiDisabled3 from '@/assets/images/api-disabled-3.png';
import apiDisabled1Dark from '@/assets/images/api-disabled-1-dark.png';
import apiDisabled2Dark from '@/assets/images/api-disabled-2-dark.png';
import apiDisabled3Dark from '@/assets/images/api-disabled-3-dark.png';

const colorMode = useColorMode();

const items = [
  {
    id: 1,
    image: apiDisabled1,
    imageDark: apiDisabled1Dark
  },
  {
    id: 2,
    image: apiDisabled2,
    imageDark: apiDisabled2Dark
  },
  {
    id: 3,
    image: apiDisabled3,
    imageDark: apiDisabled3Dark
  }
];
</script>
