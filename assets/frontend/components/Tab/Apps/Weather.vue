<template>
  <SectionCard
    data-id="apps-section-weather"
    title="Weather"
    :ui="{ title: 'font-medium' }"
  >
    <template #raw-body>
      <TabAppsGeolocation />
    </template>
  </SectionCard>
</template>
