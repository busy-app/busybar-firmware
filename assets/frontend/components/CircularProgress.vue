<template>
  <div
    class="va-progress-circle"
    :style="rootStyle"
    :class="rootClass"
  >
    <svg
      class="va-progress-circle__wrapper"
      viewBox="0 0 40 40"
    >
      <circle
        class="va-progress-circle__overlay"
        cx="50%"
        cy="50%"
        :r="radius"
        fill="none"
        :stroke="props.trackColor || 'transparent'"
        :stroke-width="cappedThickness + '%'"
        :stroke-dasharray="dasharray"
        :stroke-dashoffset="0"
      />
      <circle
        class="va-progress-circle__overlay"
        cx="50%"
        cy="50%"
        :r="radius"
        fill="none"
        :stroke="props.color"
        :stroke-width="cappedThickness + '%'"
        :stroke-dasharray="dasharray"
        :stroke-dashoffset="dashoffset"
      />
    </svg>
    <div
      v-if="$slots.default"
      :style="{ color: props.color }"
      class="va-progress-circle__info"
    >
      <slot />
    </div>
  </div>
</template>

<script lang="ts" setup>
// Retrofitted from Vuestic UI's VaProgressCircle: https://ui.vuestic.dev/ui-elements/progress-circle

const props = defineProps({
  size: { type: String, default: '24px' },
  modelValue: { type: [Number, String], default: 0 },
  indeterminate: { type: Boolean, default: false },
  thickness: { type: [Number, String], default: 0.3 },
  color: { type: String, default: '#6A7282' },
  trackColor: { type: String, default: '' }
});

const cappedThickness = computed(() => clamp(Number(props.thickness), 0, 1) / 2 * 100);

const radius = computed(() => 20 - (20 * cappedThickness.value / 100));
const dasharray = computed(() => 2 * Math.PI * radius.value);
const dashoffset = computed(() => dasharray.value * (1 - clamp(Number(props.modelValue), 0, 100) / 100));

const rootStyle = computed(() => ({
  width: props.size,
  height: props.size
}));

const rootClass = computed(() => ({
  'va-progress-circle--indeterminate': props.indeterminate
}));

const clamp = (value: number, min: number, max: number) => {
  return Math.min(Math.max(value, min), max);
};

watch(() => props.modelValue, newValue => {
  console.warn(`[VaProgressCircle]: modelValue is ${newValue}`);
  console.log(dasharray.value, dashoffset.value);
});
</script>

<style>
.va-progress-circle {
  position: relative;
  overflow: hidden;
}

.va-progress-circle__wrapper {
  position: absolute;
  top: 0;
  left: 0;
  bottom: 0;
  right: 0;
  margin: auto;
  transform: rotate(-90deg);
  width: var(--va-progress-circle-width);
  height: var(--va-progress-circle-height);

  display: flex;
  justify-content: center;
  align-items: center;
}

.va-progress-circle--indeterminate & {
  will-change: stroke-dasharray stroke-dashoffset;
  animation: va-progress-circle__wrapper--indeterminate 2s linear infinite;
}

.va-progress-circle__overlay {
  transition: var(--va-progress-circle-overlay-transition);
}

.va-progress-circle--indeterminate & {
  will-change: stroke-dasharray stroke-dashoffset;
  animation: va-progress-circle__overlay--indeterminate 2s ease-in-out infinite;
}

.va-progress-circle__info {
  position: absolute;
  left: 50%;
  top: 50%;
  transform: translate(-50%, -50%);
}

@keyframes va-progress-circle__wrapper--indeterminate {
  100% {
    transform: rotate(270deg);
  }
}

@keyframes va-progress-circle__overlay--indeterminate {
  0% {
    stroke-dasharray: 1, 125;
    stroke-dashoffset: 0;
  }

  50% {
    stroke-dasharray: 125, 125;
    stroke-dashoffset: -65px;
  }

  100% {
    stroke-dasharray: 125, 125;
    stroke-dashoffset: -125px;
  }
}
</style>
