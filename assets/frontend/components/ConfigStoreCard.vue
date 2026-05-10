<template>
  <div class="fixed right-4 bottom-4 z-50">
    <UPopover
      v-model:open="isPopoverOpen"
      :content="{
        side: 'top',
        align: 'end',
        sideOffset: 12
      }"
      :ui="{
        content: 'w-[min(28rem,calc(100vw-2rem))] rounded-2xl bg-surface-container ring-accented/75 shadow-2xl'
      }"
    >
      <UButton
        data-id="config-store-trigger"
        icon="i-bi-slider"
        label="Config"
        color="neutral"
        :variant="isPopoverOpen ? 'solid' : 'outline'"
        class="rounded-full shadow-lg"
      />

      <template #content>
        <div class="h-100 flex flex-col gap-2 p-2">
          <div class="flex items-center justify-between gap-3">
            <UInput
              v-model="searchQuery"
              data-id="config-store-search"
              placeholder="Search by name or label"
              :ui="{
                base: 'rounded-xl'
              }"
            >
              <template #leading>
                <UIcon
                  name="i-bi-search"
                  class="size-4 text-muted"
                />
              </template>
            </UInput>

            <div class="flex items-center gap-2">
              <UTooltip
                text="Open this card on page load"
                :delay-duration="0"
              >
                <UButton
                  data-id="config-store-pin"
                  :icon="configStore.openPopoverOnPageLoad ? 'i-busy-pin-fill' : 'i-busy-pin'"
                  :color="configStore.openPopoverOnPageLoad ? 'primary' : 'neutral'"
                  variant="ghost"
                  square
                  @click="configStore.openPopoverOnPageLoad = !configStore.openPopoverOnPageLoad"
                />
              </UTooltip>

              <UButton
                data-id="config-store-close"
                icon="i-bi-cross"
                color="neutral"
                variant="ghost"
                square
                @click="isPopoverOpen = false"
              />
            </div>
          </div>

          <div class="overflow-y-auto pr-1">
            <div class="flex flex-col gap-2">
              <div
                v-for="item in filteredItems"
                :key="item.name"
                class="rounded-xl border border-default/70 bg-default/70 p-3"
              >
                <div class="flex items-center justify-between gap-2">
                  <div class="flex flex-col gap-0.75 min-w-0">
                    <div class="break-all text-xs font-medium opacity-85">
                      {{ item.name }}
                    </div>
                    <div class="text-sm">
                      {{ item.label }}
                    </div>
                  </div>

                  <div class="shrink-0">
                    <USwitch
                      v-if="item.type === 'boolean'"
                      :model-value="item.value"
                      @update:model-value="updateBooleanItem(item, $event)"
                    />

                    <UInput
                      v-else-if="item.type === 'string'"
                      :model-value="item.value"
                      class="w-44"
                      @update:model-value="updateStringItem(item, $event)"
                    />

                    <UInput
                      v-else
                      :model-value="String(item.value)"
                      type="number"
                      class="w-28"
                      @update:model-value="updateNumberItem(item, $event)"
                    />
                  </div>
                </div>
              </div>

              <div
                v-if="filteredItems.length === 0"
                class="rounded-xl border border-dashed border-default/70 px-4 py-6 text-center text-sm text-muted"
              >
                No config items match your search.
              </div>
            </div>
          </div>
        </div>
      </template>
    </UPopover>
  </div>
</template>

<script setup lang="ts">
import type { ConfigStoreItem } from '../stores/configStore';

const configStore = useConfigStore();

const isPopoverOpen = ref(false);
const searchQuery = ref('');

const filteredItems = computed(() => {
  const normalizedQuery = searchQuery.value.trim().toLowerCase();

  if (normalizedQuery.length === 0) {
    return configStore.items;
  }

  return configStore.items.filter(item => {
    return item.name.toLowerCase().includes(normalizedQuery) || item.label.toLowerCase().includes(normalizedQuery);
  });
});

function updateBooleanItem (item: Extract<ConfigStoreItem, { type: 'boolean' }>, value: boolean) {
  item.value = value;
}

function updateStringItem (item: Extract<ConfigStoreItem, { type: 'string' }>, value: string | number) {
  item.value = String(value);
}

function updateNumberItem (item: Extract<ConfigStoreItem, { type: 'number' }>, value: string | number) {
  const nextValue = Number(value);

  if (Number.isNaN(nextValue)) {
    return;
  }

  item.value = nextValue;
}

onMounted(() => {
  isPopoverOpen.value = configStore.openPopoverOnPageLoad;
});
</script>
