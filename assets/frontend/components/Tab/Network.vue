<template>
  <SectionCard
    title="Wi-Fi"
    subtitle="Disconnected"
    icon="i-ri-signal-wifi-line"
  >
    <template #actions>
      <UButton
        label="Select network"
        :ui="{
          base: 'px-2.5 py-2 rounded-full'
        }"
      />
    </template>
  </SectionCard>

  <SectionCard
    title="Select network"
  >
    <template #leading-actions>
      <UButton
        icon="i-ri-arrow-left-line"
        variant="ghost"
        color="neutral"
        square
        :ui="{
          base: 'p-3 rounded-full'
        }"
      />
    </template>

    <template #actions>
      <UTooltip
        text="Add network"
        :delay-duration="0"
      >
        <UButton
          icon="i-ri-add-line"
          variant="ghost"
          color="neutral"
          square
          :ui="{
            base: 'p-3 rounded-full'
          }"
        />
      </UTooltip>
    </template>

    <template #raw-body>
      <div class="flex flex-col gap-1.5">
        <div
          v-for="i in 5"
          :key="i"
          class="flex gap-2 items-center px-2.5 py-2.5 cursor-pointer hover:bg-elevated rounded-xl"
        >
          <UIcon
            name="i-ri-signal-wifi-3-fill"
            class="size-6 text-muted"
          />
          <div class="grow">Keenetic-6436</div>
          <UIcon
            name="i-ri-lock-line"
            class="size-6 text-muted"
          />
        </div>
      </div>
    </template>
  </SectionCard>

  <SectionCard
    title="Keenetic-6436"
    icon="i-ri-signal-wifi-fill"
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

    <div class="flex justify-between items-center">
      <div>IP Address</div>
      <CopyButton
        text="192.168.1.42"
        variant="link"
        color="neutral"
        class="px-0"
      />
    </div>
    <div class="grid sm:grid-cols-2 gap-y-3 gap-x-1">
      <div
        v-for="[property, value] in Object.entries({
          'Name': 'Keenetic-6436',
          'Signal strength': '-61 dBm',
          'Channel': '6 (2.4 GHz)',
          'BSSID': 'FA:BA:BA:F0:FA:FA',
          'Security': 'WPA2',
          'Standard': '802.11ac'
        })"
        :key="property"
        class="flex"
      >
        <div class="w-[120px] text-muted">{{ property }}</div>
        <div class="max-w-[140px] md:max-w-[180px] text-ellipsis overflow-hidden">{{ value }}</div>
      </div>
    </div>
  </SectionCard>

  <SectionCard
    title="HTTP API"
    icon="i-ri-exchange-2-line"
  >
    <div class="flex justify-between items-center">
      <div>Over USB</div>
      <UButton
        variant="link"
        class="px-0"
      >
        <span class="underline">http://10.04.20/docs</span>
        <UIcon
          name="i-ri-external-link-line"
          class="size-4"
        />
      </UButton>
    </div>
    <div class="flex flex-col gap-1">
      <div>Over USB</div>
      <div class="text-sm text-muted">Connect to a Wi-Fi network to enable access to HTTP API over Wi-Fi</div>
    </div>
  </SectionCard>
</template>
