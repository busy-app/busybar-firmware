<template>
  <section
    v-if="!isHidden"
    data-id="network-section-apps-catalog-banner"
    class="relative flex min-h-20 w-full items-center overflow-hidden rounded-3xl bg-brand-950 pr-4 pl-2 ring-1 ring-glass"
  >
    <div class="pointer-events-none absolute inset-0 bg-linear-to-l from-brand-500/31 to-transparent to-35%" />
    <div class="pointer-events-none absolute -top-32.25 left-9.25 h-39 w-40.5 rounded-full bg-brand-500 blur-3xl" />
    <img
      src="~/assets/images/apps-catalog-banner/star.svg"
      alt=""
      class="pointer-events-none absolute top-1 left-3.75 max-w-none mix-blend-overlay"
    >

    <img
      src="~/assets/images/apps-catalog-banner/illustration.png"
      alt=""
      class="relative hidden h-20 w-33.25 shrink-0 sm:block"
    >

    <div class="relative flex min-w-0 flex-1 items-start gap-1.5 py-4 pl-2 sm:pl-0">
      <div class="flex min-w-0 flex-1 flex-col gap-3 sm:flex-row sm:items-center sm:gap-2">
        <div class="flex min-w-0 flex-1 flex-col gap-1">
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
