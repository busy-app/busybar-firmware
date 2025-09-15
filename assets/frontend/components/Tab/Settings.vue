<template>
  <SectionCard>
    <div class="grid sm:grid-cols-2 divide-y sm:divide-x sm:divide-y-0 divide-neutral-300/30 dark:divide-neutral-700/30 p-2">
      <div class="flex flex-col gap-8 pb-6 sm:pb-0 sm:pr-6">
        <div class="flex justify-between items-center">
          <UIcon
            name="i-ri-volume-up-line"
            class="size-7"
          />

          <UButton
            label="Mute"
            icon="i-ri-volume-mute-line"
            variant="subtle"
            size="sm"
            color="neutral"
            class="rounded-full"
          />
        </div>

        <div class="flex flex-col gap-2.5">
          <div class="flex justify-between items-center">
            <div class="text-lg font-medium">Sound</div>
            <div class="text-muted">55%</div>
          </div>

          <USlider
            :default-value="55"
            :ui="{
              root: '',
              track: 'h-[14px]',
              range: 'rounded-r-none',
              thumb: 'bg-primary ring-4 ring-white size-[6px] focus-visible:outline-none'
            }"
          />
        </div>
      </div>

      <div class="flex flex-col gap-8 pt-6 sm:pt-0 sm:pl-6">
        <div class="flex justify-between items-center">
          <UIcon
            name="i-ri-sun-line"
            class="size-7"
          />

          <UButton
            label="Auto"
            icon="i-ri-input-method-line"
            variant="subtle"
            size="sm"
            color="neutral"
            class="rounded-full"
          />
        </div>

        <div class="flex flex-col gap-2.5">
          <div class="flex justify-between items-center">
            <div class="text-lg font-medium">Brightness</div>
            <div class="text-muted">80%</div>
          </div>

          <USlider
            :default-value="80"
            :ui="{
              root: '',
              track: 'h-[14px]',
              range: 'rounded-r-none',
              thumb: 'bg-primary ring-4 ring-white size-[6px] focus-visible:outline-none'
            }"
          />
        </div>
      </div>
    </div>

    <div class="flex flex-col gap-4">
      <div class="flex items-center gap-2.5">
        <UIcon
          name="i-ri-information-fill"
          class="size-6"
        />
        <div class="font-medium">Device</div>
      </div>
      <div class="grid sm:grid-cols-2 gap-y-3 gap-x-1">
        <div
          v-for="[property, value] in Object.entries({
            'Version': 'V1.2-26070',
            'Build date': '9 Jun 2025',
            'Language': 'English',
            'Timezone': 'UTC+0'
          })"
          :key="property"
          class="flex"
        >
          <div class="w-[120px] text-muted">{{ property }}</div>
          <div class="max-w-[140px] md:max-w-[180px] text-ellipsis overflow-hidden">{{ value }}</div>
        </div>
      </div>
    </div>

    <div class="flex flex-col gap-4">
      <div class="flex items-center gap-2.5">
        <UIcon
          name="i-ri-cpu-fill"
          class="size-6"
        />
        <div class="font-medium">Hardware</div>
      </div>
      <div class="grid sm:grid-cols-2 gap-y-3 gap-x-1">
        <div
          v-for="[property, value] in Object.entries({
            'Main display resolution': '72×16 (LED)',
            'Central MCU': 'STM32U5M',
            'Main display refresh rate': '60 Hz',
            'Wireless MCU': 'Silicon Labs SiWG917',
            'RAM size': '2.5 MB'
          })"
          :key="property"
          class="flex"
        >
          <div class="w-[120px] text-muted">{{ property }}</div>
          <div class="max-w-[140px] md:max-w-[180px] text-ellipsis overflow-hidden">{{ value }}</div>
        </div>
      </div>
    </div>
  </SectionCard>
</template>
