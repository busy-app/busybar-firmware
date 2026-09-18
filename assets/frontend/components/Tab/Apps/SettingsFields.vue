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
        :fields="field.fields"
      />
    </div>

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
        class="w-36 shrink-0"
        @update:model-value="values[key] = $event"
      />

      <UInput
        v-else-if="field.type === 'string'"
        :model-value="values[key] as string"
        :type="field.sensitive ? 'password' : 'text'"
        :maxlength="field.max_length"
        class="w-56 shrink-0"
        @update:model-value="values[key] = $event"
      />

      <USelect
        v-else-if="field.type === 'enum'"
        :model-value="values[key] as string"
        :items="field.options"
        class="w-56 shrink-0"
        @update:model-value="values[key] = $event"
      />
    </div>
  </template>
</template>

<script setup lang="ts">
import type { AppSettingsNode } from '@/stores/appsStore';

defineProps<{
  fields: Record<string, AppSettingsNode>;
}>();

const values = defineModel<Record<string, unknown>>('values', { required: true });
</script>
