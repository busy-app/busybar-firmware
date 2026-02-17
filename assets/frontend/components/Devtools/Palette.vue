<template>
  <section class="flex flex-col gap-5">
    <!-- color classes (check for missing variants or wrong variables applied) -->
    <div
      class="grid auto-rows-[minmax(3rem,auto)] ring ring-glass rounded-2xl bg-white dark:bg-black"
    >
      <div
        class="grid grid-cols-[160px_1fr] px-4 py-3.5 text-sm uppercase tracking-[0.2em] text-muted border-b border-[rgba(255,255,255,0.05)]"
      >
        <div class="flex items-center">Variable</div>
        <div class="flex items-center justify-between">
          <span>{{ useColorMode().value === 'dark' ? 'Dark theme' : 'Light theme' }} - toggle manually</span>

          <UButton
            :icon="useColorMode().value === 'dark' ? 'i-bi-brightness' : 'i-bi-moon'"
            color="neutral"
            variant="ghost"
            size="xs"
            @click="useColorMode().value = useColorMode().value === 'dark' ? 'light' : 'dark'"
          />
        </div>
      </div>
      <div
        v-for="name in colorClassBases"
        :key="name"
        class="grid grid-cols-[160px_1fr] px-4 py-3 border-b border-[rgba(255,255,255,0.03)] last:border-b-0"
      >
        <p class="m-0 font-semibold text-[0.9rem] text-[var(--ui-text)] tracking-[0.12em]">
          {{ name }}
        </p>
        <div
          class="flex flex-col justify-center gap-[0.65rem] pl-2"
        >
          <div class="flex items-center gap-2">
            <div
              class="size-6 rounded flex-shrink-0"
              :class="`bg-${name}`"
            />
            <span class="text-muted font-mono text-sm">{{ `bg-${name}` }}</span>
          </div>

          <div class="flex items-center gap-2">
            <UIcon
              name="i-bi-firmware-fill"
              class="size-6"
              :class="`text-${name}`"
            />
            <span class="text-muted font-mono text-sm">{{ `text-${name}` }}</span>
          </div>
        </div>
      </div>
    </div>

    <!-- css variables (compare to figma ui kit) -->
    <div
      v-if="palette.length > 0"
      class="grid auto-rows-[minmax(3rem,auto)] ring ring-glass rounded-2xl bg-white dark:bg-black"
    >
      <div
        class="grid grid-cols-[160px_1fr_1fr] px-4 py-3.5 text-sm uppercase tracking-[0.2em] text-muted border-b border-[rgba(255,255,255,0.05)]"
      >
        <span class="text-left">Variable</span>
        <span v-for="theme in palette" :key="theme.id">
          {{ theme.label }}
        </span>
      </div>
      <div
        v-for="name in cssVariables"
        :key="name"
        class="grid grid-cols-[160px_1fr_1fr] px-4 py-3 border-b border-[rgba(255,255,255,0.03)] last:border-b-0"
      >
        <p class="m-0 font-semibold text-[0.9rem] text-[var(--ui-text)] tracking-[0.12em]">
          {{ name }}
        </p>
        <div
          v-for="theme in palette"
          :key="theme.id"
          class="flex items-center gap-[0.65rem] pl-2"
        >
          <span
            class="w-10 h-10 rounded-[0.45rem] border border-[rgba(255,255,255,0.2)] flex-shrink-0"
            :style="{
              backgroundColor:
                theme.entries[name]?.raw === 'inherit'
                  ? 'transparent'
                  : theme.entries[name]?.raw || 'transparent'
            }"
          />
          <div class="flex flex-col gap-[0.1rem] text-sm text-muted">
            <span class="font-mono font-semibold text-[var(--ui-text-highlighted)]">
              {{ theme.entries[name]?.hex || theme.entries[name]?.raw || '—' }}
            </span>
          </div>
        </div>
      </div>
    </div>
  </section>
</template>

<script setup lang="ts">
import { onMounted, ref } from 'vue';

const colorClassBases = [
  'primary',
  'secondary',
  'success',
  'info',
  'warning',
  'error',

  'dimmed',
  'muted',
  'toned',
  'default',
  'elevated',
  'accented',
  'highlighted',
  'inverted'
] as const;

const cssVariables = [
  '--ui-primary',
  '--ui-primary-text',
  '--ui-secondary',
  '--ui-success',
  '--ui-success-text',
  '--ui-success-special',
  '--ui-success-special-text',
  '--ui-info',
  '--ui-info-text',
  '--ui-warning',
  '--ui-warning-text',
  '--ui-error',
  '--ui-error-text',
  '--ui-text-dimmed',
  '--ui-text-muted',
  '--ui-text-toned',
  '--ui-text',
  '--ui-text-highlighted',
  '--ui-bg',
  '--ui-bg-elevated',
  '--ui-bg-accented',
  '--ui-bg-accented-inverted',
  '--ui-bg-inverted',
  '--ui-bg-special',
  '--ui-bg-special-inverted',
  '--ui-border',
  '--ui-border-accented',
  '--ui-border-inverted',
  '--ui-border-opaque',
  '--ui-bg-modal',
  '--ui-bg-overlay-modal',
  '--ui-content-transparent'
] as const;

type CssVariableName = (typeof cssVariables)[number];

const themeVariants = [
  { id: 'dark', label: 'Dark theme', className: 'dark' },
  { id: 'light', label: 'Light theme', className: 'light' }
] as const;

type ThemeVariant = (typeof themeVariants)[number];
type ThemeVariantId = ThemeVariant['id'];

type ThemeColor = {
  raw: string;
  hex: string;
};

type ThemePalette = {
  id: ThemeVariantId;
  label: string;
  entries: Record<CssVariableName, ThemeColor>;
};

const palette = ref<ThemePalette[]>([]);
let colorContext: CanvasRenderingContext2D | null = null;

function withThemeClass<T> (themeClass: string, fn: () => T): T {
  const root = document.documentElement;
  const previousClasses = root.className;
  root.classList.remove('light', 'dark');
  if (themeClass) {
    root.classList.add(themeClass);
  }
  try {
    return fn();
  } finally {
    root.className = previousClasses;
  }
}

function rgbaStringToHex (value: string): string | null {
  const match = value.match(/rgba?\(([^)]+)\)/);
  if (!match) {
    return null;
  }
  const channelMatches = Array.from(match[1].matchAll(/[\d.]+%?/g)).map(m => m[0]);
  if (channelMatches.length < 3) {
    return null;
  }
  const [rRaw, gRaw, bRaw, aRaw = '1'] = channelMatches;
  const r = parseChannel(rRaw);
  const g = parseChannel(gRaw);
  const b = parseChannel(bRaw);
  const alpha = clampAlpha(parseFloat(aRaw));
  const baseHex = `#${toHex(r)}${toHex(g)}${toHex(b)}`.toUpperCase();
  if (alpha < 1) {
    return `${baseHex}${toHex(Math.round(alpha * 255))}`.toUpperCase();
  }
  return baseHex;
}

function parseChannel (value: string): number {
  if (value.endsWith('%')) {
    return clampChannel(parseFloat(value) * 2.55);
  }
  return clampChannel(parseFloat(value));
}

function clampChannel (value: number): number {
  if (Number.isNaN(value)) {
    return 0;
  }
  return Math.min(255, Math.max(0, Math.round(value)));
}

function clampAlpha (value: number): number {
  if (Number.isNaN(value)) {
    return 1;
  }
  return Math.min(1, Math.max(0, value));
}

function toHex (value: number): string {
  return value.toString(16).padStart(2, '0');
}

function formatColor (value: string): string {
  if (!colorContext) {
    return value;
  }
  const candidate = value.trim() || 'transparent';
  colorContext.fillStyle = candidate;
  const canonical = colorContext.fillStyle;
  return rgbaStringToHex(canonical) ?? canonical;
}

onMounted(() => {
  colorContext = document.createElement('canvas').getContext('2d');
  palette.value = themeVariants.map(variant => withThemeClass(variant.className, () => {
    const computed = getComputedStyle(document.documentElement);
    const entries = {} as Record<CssVariableName, ThemeColor>;
    cssVariables.forEach(name => {
      const rawValue = computed.getPropertyValue(name).trim();
      const raw = rawValue || 'inherit';
      const hex = raw === 'inherit' ? raw : formatColor(raw);
      entries[name] = { raw, hex };
    });
    return {
      id: variant.id,
      label: variant.label,
      entries
    };
  }));
});
</script>

