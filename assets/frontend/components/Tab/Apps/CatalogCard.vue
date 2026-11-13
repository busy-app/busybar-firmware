<template>
  <SectionCard
    data-id="apps-section-catalog"
    title="Apps Catalog"
    subtitle="Explore free apps made by our team and community"
    icon="i-bi-apps-add"
    :ui="{ title: 'font-medium' }"
  >
    <template #actions>
      <UButton
        data-id="apps-section-catalog-open-button"
        :to="appConfig.links.appsCatalog"
        target="_blank"
        label="Open catalog"
        trailing-icon="i-ri-arrow-right-up-line"
        color="neutral"
        class="w-fit min-w-20 justify-center"
        :ui="{
          base: 'px-4 py-2.5 gap-1.5',
          trailingIcon: 'size-5'
        }"
      />
    </template>
  </SectionCard>
</template>

<script setup lang="ts">
const appConfig = useAppConfig();
</script>
