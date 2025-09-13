<template>
  <div class="w-full flex flex-col gap-4">
    <div class="w-full flex justify-between items-center ring-1 ring-glass rounded-3xl p-6 bg-elevated/50">
      <div class="flex items-center gap-4">
        <div class="size-14 rounded-full bg-elevated p-[14px]">
          <UIcon
            name="i-tabler-wifi"
            class="size-7"
          />
        </div>

        <div>
          <div class="text-xl">Wi-Fi</div>
          <div class="text-muted">Disconnected</div>
        </div>
      </div>

      <div>
        <UButton
          label="Select network"
          :ui="{
            base: 'px-2.5 py-2 rounded-full'
          }"
        />
      </div>
    </div>

    <UCard
      :ui="{
        root: 'ring-1 ring-glass rounded-3xl bg-elevated/50 divide-none',
        header: 'p-4 sm:p-6',
        body: 'p-4 sm:p-6 pt-0 sm:pt-0',
        footer: 'p-4 sm:p-6'
      }"
    >
      <template #header>
        <div class="w-full flex justify-between items-center">
          <div class="flex items-center gap-4">
            <div class="size-14 rounded-full bg-elevated p-[14px]">
              <UIcon
                name="i-tabler-wifi"
                class="size-7"
              />
            </div>

            <div>
              <div class="text-xl">Wi-Fi</div>
              <div class="text-muted">Disconnected</div>
            </div>
          </div>

          <div class="flex items-center gap-4">
            <UButton
              label="Select network"
              :ui="{
                base: 'px-2.5 py-2 rounded-full'
              }"
            />
          </div>
        </div>
      </template>

      <div class="bg-elevated/75 p-4 rounded-xl">
        <div class="grid grid-cols-2 gap-y-3">
          <div class="flex">
            <div class="w-[120px] text-muted">Version</div>
            <div>V1.2-26070</div>
          </div>
          <div class="flex">
            <div class="w-[120px] text-muted">Build date</div>
            <div>9 Jun 2025</div>
          </div>
          <div class="flex">
            <div class="w-[120px] text-muted">Language</div>
            <div>English</div>
          </div>
          <div class="flex">
            <div class="w-[120px] text-muted">Timezone</div>
            <div>UTC+0</div>
          </div>
        </div>
      </div>
    </UCard>

    <SectionCard
      title="Wi-Fi"
      icon="i-tabler-wifi"
    >
      <template #description>
        <div class="text-muted">Disconnected</div>
      </template>

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
      title="Wi-Fi"
      icon="i-tabler-wifi"
    >
      <template #description>
        <div class="text-muted">Disconnected</div>
      </template>

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
      title="Wi-Fi"
      icon="i-tabler-wifi"
    >
      <template #description>
        <div class="text-muted">Disconnected</div>
      </template>

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
  </div>
</template>

<script setup lang="ts">
useHead({
  title: 'BUSY Bar Virtual LAN',
  meta: [
    {
      name: 'description',
      content: 'Control your BUSY Bar in the browser'
    }
  ]
});
</script>
