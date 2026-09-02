<template>
  <SectionCard
    data-id="apps-section-your-apps"
    title="Your apps"
    :ui="{ title: 'font-medium' }"
  >
    <template #raw-body>
      <div class="grid grid-cols-[repeat(auto-fit,minmax(min(100%,19.5rem),1fr))] gap-4">
        <slot />
      </div>
    </template>
  </SectionCard>
</template>
