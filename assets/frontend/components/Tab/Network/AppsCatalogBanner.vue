<template>
  <section
    v-if="!isHidden"
    data-id="network-section-apps-catalog-banner"
    class="relative w-full min-h-20 flex items-center pl-2 pr-4 overflow-hidden rounded-3xl ring-1 ring-glass bg-brand-950"
  >
    <div class="absolute inset-0 pointer-events-none bg-linear-to-l from-brand-500/31 to-transparent to-35%" />
    <img
      src="~/assets/images/apps-catalog-banner/glow-top.svg"
      alt=""
      class="absolute -left-26.75 -top-68.25 max-w-none pointer-events-none"
    >
    <img
      src="~/assets/images/apps-catalog-banner/star.svg"
      alt=""
      class="absolute left-3.75 top-1 max-w-none mix-blend-overlay pointer-events-none"
    >
    <img
      src="~/assets/images/apps-catalog-banner/glow-bottom.svg"
      alt=""
      class="absolute -left-23.75 top-3.5 max-w-none pointer-events-none"
    >

    <img
      src="~/assets/images/apps-catalog-banner/illustration.svg"
      alt=""
      class="relative hidden sm:block shrink-0"
    >

    <div class="relative flex-1 min-w-0 flex items-start gap-1.5 py-4 pl-2 sm:pl-0">
      <div class="flex-1 min-w-0 flex flex-col sm:flex-row sm:items-center gap-3 sm:gap-2">
        <div class="flex-1 min-w-0 flex flex-col gap-1">
          <div class="flex items-center gap-2">
            <h2 class="font-medium text-white">Apps Catalog is here</h2>
            <UBadge
              label="New"
              class="bg-white/20 text-white"
            />
          </div>
          <p class="text-sm text-white/70">Explore free apps made by our team and community</p>
        </div>

        <UButton
          data-id="network-section-apps-catalog-banner-open-button"
          :to="appConfig.links.appsCatalog"
          target="_blank"
          label="Open catalog"
          trailing-icon="i-ri-arrow-right-up-line"
          color="neutral"
          class="w-fit min-w-20 justify-center bg-white text-neutral-900 hover:bg-white/90 active:bg-white/90 dark:bg-white dark:text-neutral-900 dark:hover:bg-white/90 dark:active:bg-white/90"
          :ui="{
            base: 'px-3.5 py-2 gap-1.5',
            trailingIcon: 'size-5'
          }"
        />
      </div>

      <UTooltip
        text="Hide"
        :delay-duration="0"
      >
        <UButton
          data-id="network-section-apps-catalog-banner-hide-button"
          icon="i-bi-cross"
          aria-label="Hide banner"
          color="neutral"
          variant="link"
          class="text-white/50 hover:text-white"
          :ui="{
            base: 'p-1',
            leadingIcon: 'size-4'
          }"
          @click="hideBanner"
        />
      </UTooltip>
    </div>
  </section>
</template>

<script setup lang="ts">
const HIDDEN_STORAGE_KEY = 'apps-catalog-banner-hidden';

const appConfig = useAppConfig();

const isHidden = ref(readHidden());

function readHidden () {
  try {
    return localStorage.getItem(HIDDEN_STORAGE_KEY) === 'true';
  } catch {
    return false;
  }
}

function hideBanner () {
  isHidden.value = true;

  try {
    localStorage.setItem(HIDDEN_STORAGE_KEY, 'true');
  } catch (error) {
    console.error('Failed to save apps catalog banner state:', error);
  }
}
</script>
