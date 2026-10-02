<template>
  <template
    v-for="(field, key) in fields"
    :key="key"
  >
    <div
      v-if="field.type === 'group'"
      :data-id="`app-settings-group-${key}`"
      class="flex flex-col gap-1"
    >
      <div class="px-1 pt-3 pb-1">
        <div class="font-medium">{{ field.label }}</div>
        <div
          v-if="field.description"
          class="text-sm text-muted"
        >
          {{ field.description }}
        </div>
      </div>

      <SettingsFields
        v-model:values="values[key] as Record<string, unknown>"
        :app-id="appId"
        :fields="field.fields"
      />
    </div>

    <TabAppsSettingsGeolocation
      v-else-if="field.type === 'geolocation'"
      :model-value="values[key] as AppSettingsGeolocationValue"
      :data-id="`app-settings-field-${key}`"
      :label="field.label"
      :default-value="field.default"
      class="mb-5 last:mb-0"
      @update:model-value="values[key] = $event"
    />

    <div
      v-else
      :data-id="`app-settings-field-${key}`"
      class="flex items-center gap-4 rounded-xl bg-accented/25 p-4 dark:bg-elevated/75"
    >
      <div class="min-w-0 flex-1">
        <div class="truncate">{{ field.label }}</div>
        <div
          v-if="field.description"
          class="text-sm text-muted"
        >
          {{ field.description }}
        </div>
        <div
          v-if="errors[key]"
          :data-id="`app-settings-field-${key}-error`"
          class="text-sm text-error"
        >
          {{ errors[key] }}
        </div>
      </div>

      <USwitch
        v-if="field.type === 'boolean'"
        :model-value="values[key] as boolean"
        @update:model-value="values[key] = $event"
      />

      <UInputNumber
        v-else-if="field.type === 'integer'"
        :model-value="values[key] as number"
        :min="field.min"
        :max="field.max"
        :step="field.step"
        :increment="NUMBER_BUTTON_PROPS"
        :decrement="NUMBER_BUTTON_PROPS"
        :ui="INPUT_UI"
        :style="GLASS_CONTROL_STYLE"
        class="w-36 shrink-0"
        @update:model-value="values[key] = $event"
      />

      <UInput
        v-else-if="field.type === 'string'"
        :model-value="values[key] as string"
        :type="field.sensitive ? 'password' : 'text'"
        :color="errors[key] ? 'error' : undefined"
        :highlight="!!errors[key]"
        :ui="errors[key] ? { base: 'focus-visible:outline-2 focus-visible:outline-error' } : INPUT_UI"
        :style="errors[key] ? { ...GLASS_CONTROL_STYLE, borderColor: 'var(--ui-error)' } : GLASS_CONTROL_STYLE"
        class="w-56 shrink-0"
        @update:model-value="values[key] = $event"
      />

      <USelect
        v-else-if="field.type === 'enum'"
        :model-value="values[key] as string"
        :items="field.options"
        variant="soft"
        size="sm"
        :style="GLASS_CONTROL_STYLE"
        class="h-7 w-fit min-w-[53px] shrink-0"
        :ui="{
          base: 'text-sm leading-4.5 ps-2 pe-2 py-1 gap-1 focus-visible:outline-2 focus-visible:outline-primary',
          trailing: 'static p-0 pe-0',
          trailingIcon: 'size-3',
          content: 'min-w-max',
          label: 'text-base',
          item: 'text-base',
          viewport: '[&::-webkit-scrollbar]:w-1 [&::-webkit-scrollbar-thumb]:rounded-full [&::-webkit-scrollbar-thumb]:bg-accented'
        }"
        @update:model-value="values[key] = $event"
      />

      <UPopover
        v-else-if="field.type === 'color'"
        :ui="{ content: 'rounded-xl bg-surface-container ring-accented/75' }"
      >
        <UButton
          :label="values[key] as string"
          color="neutral"
          variant="outline"
          class="shrink-0 font-mono"
        >
          <template #leading>
            <span
              class="size-5 rounded-md ring-1 ring-default"
              :style="{ backgroundColor: values[key] as string }"
            />
          </template>
        </UButton>

        <template #content>
          <ColorPicker
            :model-value="values[key] as string"
            format="hex"
            class="p-3"
            @update:model-value="values[key] = $event"
          />
        </template>
      </UPopover>

      <UInput
        v-else-if="field.type === 'time'"
        :model-value="values[key] as string"
        type="time"
        :step="(values[key] as string).length > 5 ? 1 : 60"
        :ui="INPUT_UI"
        :style="GLASS_CONTROL_STYLE"
        class="w-36 shrink-0"
        @update:model-value="values[key] = $event"
      />
    </div>
  </template>
</template>

<script setup lang="ts">
import type { AppSettingsGeolocationValue, AppSettingsNode } from '@/stores/appsStore';
import { GLASS_CONTROL_STYLE } from '@/util/formControlStyles';

const INPUT_UI = { base: 'focus-visible:outline-2 focus-visible:outline-primary' };
const NUMBER_BUTTON_PROPS = {
  color: 'neutral',
  variant: 'link'
} as const;

const props = defineProps<{
  appId: string;
  fields: Record<string, AppSettingsNode>;
}>();

const values = defineModel<Record<string, unknown>>('values', { required: true });

const errors = computed(() => {
  const result: Record<string, string | undefined> = {};

  for (const [key, field] of Object.entries(props.fields)) {
    if (field.type === 'string') {
      result[key] = getStringSettingError(field, values.value[key]);
    }
  }

  return result;
});
</script>
