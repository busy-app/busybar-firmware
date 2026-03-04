<template>
  <div class="flex flex-col gap-4">
    <div class="w-full text-center text-2xl text-highlighted mb-2">Device control over Wi-Fi is disabled</div>

    <div class="w-full text-center">To control your BUSY Bar via Wi-Fi, you must provide access to HTTP API.</div>

    <div class="flex flex-col lg:flex-row gap-4 pt-14">
      <div
        v-for="item in items"
        :key="item.id"
        class="flex flex-col gap-2"
      >
        <div class="w-84 flex items-center justify-center">
          <img
            :src="colorMode.value === 'dark' ? item.imageDark : item.image"
            class="w-[320px]"
          >
        </div>
        <div class="flex items-center gap-3">
          <div class="flex items-center justify-center size-11 shrink-0 rounded-full bg-accented/50 text-xl">{{ item.id }}</div>
          <div class="max-w-64">{{ item.description }}</div>
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
    description: 'Connect the device to your computer using a USB cable',
    image: apiDisabled1,
    imageDark: apiDisabled1Dark
  },
  {
    id: 2,
    description: 'Enter 10.0.4.20 in the address bar of your browser',
    image: apiDisabled2,
    imageDark: apiDisabled2Dark
  },
  {
    id: 3,
    description: 'In the Virtual LAN interface that opens, enable HTTP API access',
    image: apiDisabled3,
    imageDark: apiDisabled3Dark
  }
];
</script>
