<template>
  <SectionCard>
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
</template>
