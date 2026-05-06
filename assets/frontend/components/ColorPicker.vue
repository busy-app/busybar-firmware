<template>
  <div
    v-bind="attrs"
    class="flex flex-col gap-3"
  >
    <div class="flex gap-4">
      <UColorPicker
        :model-value="opaqueHex"
        format="hex"
        :throttle="throttle"
        :disabled="disabled"
        :size="size"
        :ui="ui"
        @update:model-value="handleOpaqueColorUpdate"
      />

      <div class="flex flex-col items-center gap-2">
        <div
          ref="alphaTrackRef"
          class="color-picker-alpha-track relative rounded-md ring-1 ring-default/70 touch-none"
          :class="[alphaTrackSizeClass, ui?.track]"
          :data-disabled="disabled ? true : undefined"
        >
          <div class="color-picker-alpha-checker absolute inset-0 rounded-md" />
          <div
            class="pointer-events-none absolute inset-0 rounded-md"
            :style="alphaTrackStyle"
          />

          <div
            aria-label="Opacity"
            class="color-picker-alpha-thumb absolute z-10"
            :class="ui?.trackThumb"
            :style="alphaThumbStyle"
            :data-disabled="disabled ? true : undefined"
          >
            <div class="color-picker-alpha-checker absolute inset-0 rounded-full" />
            <div
              class="absolute inset-0 rounded-full"
              :style="alphaThumbFillStyle"
            />
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { computed, onBeforeUnmount, onMounted, ref, useAttrs, watch } from 'vue';

defineOptions({
  inheritAttrs: false
});

type ColorPickerFormat = 'hex' | 'rgba';
type ColorPickerSize = 'xs' | 'sm' | 'md' | 'lg' | 'xl';

interface ParsedColor {
  red: number;
  green: number;
  blue: number;
  alpha: number;
}

const DEFAULT_COLOR: ParsedColor = {
  red: 255,
  green: 255,
  blue: 255,
  alpha: 1
};

const ALPHA_TRACK_SIZE_CLASSES: Record<ColorPickerSize, string> = {
  xs: 'h-38 w-[8px]',
  sm: 'h-40 w-[8px]',
  md: 'h-42 w-[8px]',
  lg: 'h-44 w-[8px]',
  xl: 'h-46 w-[8px]'
};

interface PointerPosition {
  x: number;
  y: number;
}

const props = withDefaults(defineProps<{
  defaultValue?: string;
  format?: ColorPickerFormat;
  throttle?: number;
  disabled?: boolean;
  size?: ColorPickerSize;
  ui?: Record<string, string | undefined>;
}>(), {
  defaultValue: '#FFFFFF',
  format: undefined,
  throttle: 50,
  disabled: false,
  size: 'md',
  ui: undefined
});

const model = defineModel<string>();
const attrs = useAttrs();

const opaqueHex = ref('#FFFFFF');
const alphaPercent = ref(100);
const alphaTrackRef = ref<HTMLDivElement | null>(null);
const alphaThumbPosition = ref<PointerPosition>({ x: 0, y: 0 });
const alphaDragDelta = ref<PointerPosition>();
let emitAlphaTimeout: ReturnType<typeof setTimeout> | undefined;

function handleAlphaPointerMove (event: PointerEvent) {
  moveAlphaThumb(event);
}

function handleAlphaPointerUp () {
  stopAlphaDrag();
}

const resolvedFormat = computed<ColorPickerFormat>(() => {
  if (props.format) {
    return props.format;
  }

  return inferFormat(model.value ?? props.defaultValue);
});

const alphaTrackSizeClass = computed(() => ALPHA_TRACK_SIZE_CLASSES[props.size]);
const alphaTrackStyle = computed(() => {
  const opaqueColor = parseColor(opaqueHex.value) ?? DEFAULT_COLOR;

  return {
    backgroundImage: `linear-gradient(180deg, ${formatRgba({ ...opaqueColor, alpha: 1 })} 0%, ${formatRgba({ ...opaqueColor, alpha: 0 })} 100%)`
  };
});
const currentAlpha = computed(() => clamp(alphaPercent.value / 100, 0, 1));
const alphaThumbFillStyle = computed(() => ({
  backgroundColor: formatRgba({ ...(parseColor(opaqueHex.value) ?? DEFAULT_COLOR), alpha: currentAlpha.value })
}));
const alphaThumbStyle = computed(() => ({
  top: `${alphaThumbPosition.value.y}%`
}));

watch(
  () => model.value,
  value => {
    syncFromExternalValue(value);
  },
  { immediate: true }
);

watch(
  () => props.defaultValue,
  () => {
    if (model.value === undefined) {
      syncFromExternalValue(undefined);
    }
  }
);

watch(
  () => alphaPercent.value,
  value => {
    alphaThumbPosition.value = {
      x: 0,
      y: normalizeAlpha(value)
    };
  },
  { immediate: true }
);

onMounted(() => {
  alphaTrackRef.value?.addEventListener('pointerdown', startAlphaDrag);
  window.addEventListener('pointermove', handleAlphaPointerMove);
  window.addEventListener('pointerup', handleAlphaPointerUp);
});

onBeforeUnmount(() => {
  alphaTrackRef.value?.removeEventListener('pointerdown', startAlphaDrag);
  window.removeEventListener('pointermove', handleAlphaPointerMove);
  window.removeEventListener('pointerup', handleAlphaPointerUp);

  if (emitAlphaTimeout) {
    clearTimeout(emitAlphaTimeout);
  }
});

function handleOpaqueColorUpdate (value: string | undefined) {
  const parsedColor = parseColor(value) ?? parseColor(opaqueHex.value) ?? DEFAULT_COLOR;
  const nextOpaqueHex = formatHex(parsedColor);
  const nextAlpha = currentAlpha.value <= 0 && nextOpaqueHex !== opaqueHex.value ? 1 : currentAlpha.value;

  opaqueHex.value = nextOpaqueHex;

  if (nextAlpha === 1 && alphaPercent.value !== 100) {
    alphaPercent.value = 100;
  }

  emitCurrentColorWithAlpha(nextAlpha);
}

function syncFromExternalValue (value: string | undefined) {
  const parsedColor = parseColor(value ?? props.defaultValue) ?? parseColor(props.defaultValue) ?? DEFAULT_COLOR;

  opaqueHex.value = formatHex(parsedColor);
  alphaPercent.value = Math.round(parsedColor.alpha * 100);
}
function emitCurrentColorWithAlpha (alpha: number) {
  const parsedOpaqueColor = parseColor(opaqueHex.value) ?? DEFAULT_COLOR;

  model.value = serializeColor({
    ...parsedOpaqueColor,
    alpha: clamp(alpha, 0, 1)
  }, resolvedFormat.value);
}

function startAlphaDrag (event: PointerEvent) {
  if (props.disabled) {
    event.preventDefault();
    return;
  }

  const container = alphaTrackRef.value;

  if (!container) {
    return;
  }

  const containerRect = container.getBoundingClientRect();

  alphaDragDelta.value = {
    x: containerRect.left - container.scrollLeft,
    y: containerRect.top - container.scrollTop
  };

  moveAlphaThumb(event);
}

function moveAlphaThumb (event: PointerEvent) {
  if (!alphaDragDelta.value) {
    return;
  }

  const container = alphaTrackRef.value;

  if (!container) {
    return;
  }

  const y = clamp(((event.clientY - alphaDragDelta.value.y) / container.scrollHeight) * 100, 0, 100);

  alphaThumbPosition.value = {
    x: 0,
    y
  };

  alphaPercent.value = Math.round(100 - y);

  scheduleAlphaEmit();
}

function stopAlphaDrag () {
  alphaDragDelta.value = undefined;
}

function scheduleAlphaEmit () {
  if (emitAlphaTimeout) {
    clearTimeout(emitAlphaTimeout);
  }

  emitAlphaTimeout = setTimeout(() => {
    emitCurrentColorWithAlpha(1 - (alphaThumbPosition.value.y / 100));
  }, props.throttle);
}

function normalizeAlpha (value: number) {
  return 100 - clamp(Math.round(value), 0, 100);
}

function inferFormat (value: string | undefined): ColorPickerFormat {
  const normalizedValue = value?.trim().toLowerCase() ?? '';

  if (normalizedValue.startsWith('rgba(') || normalizedValue.startsWith('rgb(')) {
    return 'rgba';
  }

  return 'hex';
}

function parseColor (value: string | undefined) {
  if (!value) {
    return null;
  }

  const normalizedValue = value.trim();

  if (normalizedValue.startsWith('#')) {
    return parseHexColor(normalizedValue);
  }

  if (/^rgba?\(/i.test(normalizedValue)) {
    return parseRgbaColor(normalizedValue);
  }

  return null;
}

function parseHexColor (value: string) {
  const normalizedValue = value.slice(1);

  if (![3, 4, 6, 8].includes(normalizedValue.length)) {
    return null;
  }

  const expandedValue = normalizedValue.length <= 4
    ? normalizedValue.split('').map(character => `${character}${character}`).join('')
    : normalizedValue;

  const channelValues = expandedValue.match(/.{2}/g);

  if (!channelValues || channelValues.length < 3) {
    return null;
  }

  const [redHex, greenHex, blueHex, alphaHex = 'FF'] = channelValues;
  const red = Number.parseInt(redHex, 16);
  const green = Number.parseInt(greenHex, 16);
  const blue = Number.parseInt(blueHex, 16);
  const alpha = Number.parseInt(alphaHex, 16) / 255;

  if ([red, green, blue, alpha].some(channelValue => Number.isNaN(channelValue))) {
    return null;
  }

  return {
    red,
    green,
    blue,
    alpha: clamp(alpha, 0, 1)
  };
}

function parseRgbaColor (value: string) {
  const matches = value.match(/^rgba?\(\s*([\d.]+)\s*,\s*([\d.]+)\s*,\s*([\d.]+)(?:\s*,\s*([\d.]+))?\s*\)$/i);

  if (!matches) {
    return null;
  }

  const [, redValue, greenValue, blueValue, alphaValue] = matches;
  const red = clamp(Math.round(Number.parseFloat(redValue)), 0, 255);
  const green = clamp(Math.round(Number.parseFloat(greenValue)), 0, 255);
  const blue = clamp(Math.round(Number.parseFloat(blueValue)), 0, 255);
  const alpha = clamp(Number.parseFloat(alphaValue ?? '1'), 0, 1);

  if ([red, green, blue, alpha].some(channelValue => Number.isNaN(channelValue))) {
    return null;
  }

  return {
    red,
    green,
    blue,
    alpha
  };
}

function serializeColor (color: ParsedColor, format: ColorPickerFormat) {
  if (format === 'rgba') {
    return formatRgba(color);
  }

  return formatHex(color, color.alpha < 1);
}

function formatHex (color: ParsedColor, includeAlpha = false) {
  const red = formatHexChannel(color.red);
  const green = formatHexChannel(color.green);
  const blue = formatHexChannel(color.blue);
  const alpha = formatHexChannel(Math.round(clamp(color.alpha, 0, 1) * 255));

  return `#${red}${green}${blue}${includeAlpha ? alpha : ''}`;
}

function formatRgba (color: ParsedColor) {
  const alpha = Number(clamp(color.alpha, 0, 1).toFixed(3));

  return `rgba(${color.red}, ${color.green}, ${color.blue}, ${alpha})`;
}

function formatHexChannel (value: number) {
  return clamp(Math.round(value), 0, 255).toString(16).padStart(2, '0').toUpperCase();
}

function clamp (value: number, minimum: number, maximum: number) {
  return Math.min(Math.max(value, minimum), maximum);
}
</script>

<style scoped>
.color-picker-alpha-checker {
  background-color: rgb(255 255 255 / 0.85);
  background-image:
    linear-gradient(45deg, rgb(15 23 42 / 0.16) 25%, transparent 25%),
    linear-gradient(-45deg, rgb(15 23 42 / 0.16) 25%, transparent 25%),
    linear-gradient(45deg, transparent 75%, rgb(15 23 42 / 0.16) 75%),
    linear-gradient(-45deg, transparent 75%, rgb(15 23 42 / 0.16) 75%);
  background-position: 0 0, 0 4px, 4px -4px, -4px 0;
  background-size: 8px 8px;
}

.color-picker-alpha-thumb {
  transform: translateY(-50%) translateX(-4px);
  width: 1rem;
  height: 1rem;
  border-radius: 9999px;
  cursor: pointer;
  box-shadow: 0 0 0 2px rgb(255 255 255);
  overflow: hidden;
}

.color-picker-alpha-track[data-disabled] .color-picker-alpha-thumb,
.color-picker-alpha-thumb[data-disabled] {
  cursor: not-allowed;
}
</style>
