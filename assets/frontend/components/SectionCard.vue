<template>
  <UCard
    :ui="resolvedCardUi"
  >
    <template
      v-if="headerExists"
      #header
    >
      <div class="w-full flex flex-col sm:flex-row justify-between items-start sm:items-center gap-4">
        <div :class="resolvedUi.titleWrapper">
          <slot name="leading-actions" />

          <div
            v-if="icon"
            :class="resolvedUi.iconWrapper"
          >
            <UIcon
              :name="icon"
              :class="resolvedUi.icon"
            />
          </div>

          <div>
            <div :class="resolvedUi.title">{{ title }}</div>
            <div
              v-if="subtitle && !$slots.subtitle"
              :class="resolvedUi.subtitle"
            >
              {{ subtitle }}
            </div>
            <slot name="subtitle" />
          </div>
        </div>

        <div :class="resolvedUi.actionsWrapper">
          <slot name="actions" />
        </div>
      </div>
    </template>

    <template
      v-if="bodyExists"
      #default
    >
      <div
        v-if="!isEmptySlot('default')"
        class="flex flex-col gap-1 rounded-group"
        :class="headerExists ? '' : 'mt-4 sm:mt-6'"
      >
        <div
          v-for="(child, i) in $slots.default!()"
          :key="child.key ?? i"
          class="bg-accented/25 dark:bg-elevated/75 p-4 rounded-xl"
        >
          <component :is="child" />
        </div>
      </div>

      <template v-if="$slots['raw-body']">
        <slot name="raw-body" />
      </template>
    </template>
  </UCard>
</template>

<script setup lang="ts">
import { twMerge } from 'tailwind-merge';

interface CardUi {
  root?: string;
  header?: string;
  body?: string;
  footer?: string;
}

interface SectionCardCustomUi {
  titleWrapper?: string;
  icon?: string;
  iconWrapper?: string;
  title?: string;
  subtitle?: string;
  actionsWrapper?: string;
}

type SectionCardUi = Partial<CardUi & SectionCardCustomUi>;

const DEFAULT_CARD_UI: Required<CardUi> = {
  root: 'ring-1 ring-glass rounded-3xl bg-elevated/50 divide-none',
  header: 'p-4 sm:p-6',
  body: 'p-4 sm:p-6 pt-0 sm:pt-0',
  footer: 'p-4 sm:p-6'
};

const DEFAULT_UI: Required<SectionCardCustomUi> = {
  titleWrapper: 'flex items-center gap-4',
  icon: 'size-7',
  iconWrapper: 'size-14 rounded-full bg-accented/25 dark:bg-elevated p-[14px]',
  title: 'text-xl',
  subtitle: 'text-muted',
  actionsWrapper: 'w-full sm:w-fit flex flex-col sm:flex-row items-stretch sm:items-end gap-2'
};

const props = withDefaults(defineProps<{
  title?: string;
  subtitle?: string;
  icon?: string;
  ui?: SectionCardUi;
}>(), {
  title: undefined,
  subtitle: undefined,
  icon: undefined,
  ui: undefined
});

const slots = useSlots();
const resolvedCardUi = computed<CardUi>(() => ({
  root: mergeUiClass(DEFAULT_CARD_UI.root, props.ui?.root),
  header: mergeUiClass(DEFAULT_CARD_UI.header, props.ui?.header),
  body: mergeUiClass(DEFAULT_CARD_UI.body, props.ui?.body),
  footer: mergeUiClass(DEFAULT_CARD_UI.footer, props.ui?.footer)
}));
const resolvedUi = computed<Required<SectionCardCustomUi>>(() => ({
  titleWrapper: mergeUiClass(DEFAULT_UI.titleWrapper, props.ui?.titleWrapper),
  icon: mergeUiClass(DEFAULT_UI.icon, props.ui?.icon),
  iconWrapper: mergeUiClass(DEFAULT_UI.iconWrapper, props.ui?.iconWrapper),
  title: mergeUiClass(DEFAULT_UI.title, props.ui?.title),
  subtitle: mergeUiClass(DEFAULT_UI.subtitle, props.ui?.subtitle),
  actionsWrapper: mergeUiClass(DEFAULT_UI.actionsWrapper, props.ui?.actionsWrapper)
}));

const headerExists = computed(() => !!(props.title || props.subtitle || props.icon || !isEmptySlot('subtitle') || !isEmptySlot('leading-actions') || !isEmptySlot('actions')));
const bodyExists = computed(() => isEmptySlot('default') === false || isEmptySlot('raw-body') === false);

function mergeUiClass (defaultClass: string, overrideClass?: string): string {
  return twMerge(defaultClass, overrideClass);
}

function isEmptySlot (slotName: string): boolean {
  if (!slots[slotName]) {
    return true;
  }
  const slot = slots[slotName]?.();
  return !(slot && slot.length && slot.some(vnode => {
    if (vnode.children === 'v-if') {
      return false;
    }
    if (typeof vnode.type === 'object' || typeof vnode.type === 'function') {
      return true;
    }
    if (typeof vnode.children === 'string') {
      return vnode.children.length > 0;
    }
    return Array.isArray(vnode.children) && vnode.children.length > 0;
  }));
}
</script>
