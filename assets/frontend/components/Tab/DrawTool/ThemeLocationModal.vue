<template>
  <ModalIllustrated
    v-model:open="open"
    data-id="modal-draw-tool-theme-location"
    title="Where to find it"
    description="Go to BUSY/CUSTOM → Setup → Theme on BUSY Bar."
  >
    <template #image>
      <figure class="flex h-55 items-center justify-center overflow-hidden rounded-xl bg-surface-container">
        <img
          :src="themeLocationImage"
          alt="BUSY Bar mode switch set to BUSY"
          width="316"
          height="138"
        >
      </figure>
    </template>

    <template #body-description>
      Go to <strong>BUSY/CUSTOM</strong> → <strong>Setup</strong> → <strong>Theme</strong> on BUSY Bar.
    </template>

    <template #actions>
      <footer class="flex items-center justify-between gap-2">
        <UCheckbox
          v-model="shouldHideHint"
          data-id="modal-draw-tool-theme-location-hide"
          label="Don’t show this again"
          color="neutral"
          size="xl"
          icon="i-bi-checkmark"
          :ui="{
            root: 'items-center',
            container: 'h-5',
            base: 'rounded ring-2 ring-neutral-500 data-[state=checked]:ring-0',
            wrapper: 'text-sm',
            label: 'text-muted'
          }"
        />
        <UButton
          data-id="modal-draw-tool-theme-location-primary-action"
          label="Got it"
          color="neutral"
          size="lg"
          class="min-w-20 justify-center"
          @click="() => { open = false; }"
        />
      </footer>
    </template>
  </ModalIllustrated>
</template>

<script setup lang="ts">
import themeLocationImage from '@/assets/images/theme-location-bar.svg';

const open = defineModel<boolean>('open', { default: false });

const dts = useDrawToolStore();

const shouldHideHint = ref(false);

watch(open, isOpen => {
  if (isOpen) {
    shouldHideHint.value = false;
    return;
  }

  if (shouldHideHint.value) {
    dts.isThemeLocationHintHidden = true;
  }
});
</script>
