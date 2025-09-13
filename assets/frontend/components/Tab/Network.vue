<template>
  <!-- <SectionCard
    title="Keenetic-6436"
    icon="i-tabler-wifi"
  >
    <template #subtitle>
      <div class="flex items-center gap-2 text-green-500">
        <div class="relative top-[-1px] size-2.5 rounded-full bg-green-500" />
        Connected
      </div>
    </template>

    <template #actions>
      <UButton
        label="Forget network"
        variant="soft"
        color="error"
        :ui="{
          base: 'px-2.5 py-2 rounded-full'
        }"
      />
    </template>
  </SectionCard> -->

  <SectionCard
    title="Wi-Fi"
    subtitle="Disconnected"
    icon="i-tabler-wifi"
  >
    <template #actions>
      <UButton
        label="Select network"
        :ui="{
          base: 'px-2.5 py-2 rounded-full'
        }"
      />
    </template>

    <div class="grid grid-cols-2 gap-y-3">
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
        <div>{{ value }}</div>
      </div>
    </div>
  </SectionCard>

  <SectionCard
    title="HTTP API"
    icon="i-tabler-wifi-off"
  >
    <div class="flex justify-between items-center">
      <div>Over USB</div>
      <UButton
        variant="link"
        trailing-icon="i-tabler-external-link"
        class="px-0"
      >
        <span class="underline">http://10.04.20/docs</span>
      </UButton>
    </div>
    <div class="flex flex-col gap-1">
      <div>Over USB</div>
      <div class="text-sm text-muted">Connect to a Wi-Fi network to enable access to HTTP API over Wi-Fi</div>
    </div>
  </SectionCard>
</template>
