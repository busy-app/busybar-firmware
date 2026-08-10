<template>
  <div class="fixed right-4 bottom-4 z-50">
    <UPopover
      v-model:open="isPopoverOpen"
      :dismissible="!configStore.pinPopover"
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
        :class="isPopoverOpen ? '' : 'bg-default'"
      />

      <template #content>
        <div class="h-100 max-h-[calc(100vh-5rem)] flex flex-col gap-2 p-2">
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
                text="Pin this card and open it on page load"
                :delay-duration="0"
              >
                <UButton
                  data-id="config-store-pin"
                  :icon="configStore.pinPopover ? 'i-busy-pin-fill' : 'i-busy-pin'"
                  :color="configStore.pinPopover ? 'primary' : 'neutral'"
                  variant="ghost"
                  square
                  @click="() => { configStore.pinPopover = !configStore.pinPopover; }"
                />
              </UTooltip>

              <UButton
                data-id="config-store-close"
                icon="i-bi-cross"
                color="neutral"
                variant="ghost"
                square
                @click="() => { isPopoverOpen = false; }"
              />
            </div>
          </div>

          <div v-if="showReloadPrompt" class="rounded-xl flex items-center justify-between gap-2">
            <div class="text-sm pl-3">
              Changes saved. Reload the page to apply
            </div>
            <UButton
              data-id="config-store-reload"
              label="Reload"
              icon="i-bi-arrow-clockwise"
              color="primary"
              size="sm"
              @click="reload"
            />
          </div>

          <div class="overflow-y-auto">
            <div class="flex flex-col gap-1">
              <div
                v-for="item in filteredLocaleSortedItemsWithChangedFirst"
                :key="item.name"
                class="rounded-xl ring ring-inset ring-default/70 bg-default/70 p-3"
                :class="item.value !== item.default ? 'bg-warning/5 ring-warning/50' : ''"
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

                  <div class="flex items-center justify-end gap-3 shrink-0">
                    <USwitch
                      v-if="item.type === 'boolean'"
                      :model-value="item.value"
                      @update:model-value="updateBooleanItem(item, $event)"
                    />

                    <UInput
                      v-else-if="item.type === 'string'"
                      v-model.lazy="item.value"
                      class="w-44"
                      @change="saveChangesToLocalStorage"
                    />

                    <UInput
                      v-else
                      v-model.lazy="item.value"
                      type="number"
                      class="w-28"
                      @change="saveChangesToLocalStorage"
                    />

                    <div
                      v-if="item.value !== item.default"
                      class="relative"
                    >
                      <div class="absolute -top-3.5 w-px h-[calc(100%+1.75rem)] bg-warning/50" />
                      <UButton
                        data-id="config-store-reset"
                        icon="i-bi-arrow-counterclockwise"
                        color="neutral"
                        variant="ghost"
                        class="relative -right-1.5"
                        square
                        @click="undoChangesToItem(item)"
                      />
                    </div>
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

const filteredLocaleSortedItems = computed(() => {
  return [...filteredItems.value].sort((a, b) => {
    return a.name.localeCompare(b.name);
  });
});

const filteredLocaleSortedItemsWithChangedFirst = computed(() => {
  const changedItems = filteredLocaleSortedItems.value.filter(item => item.value !== item.default);
  const unchangedItems = filteredLocaleSortedItems.value.filter(item => item.value === item.default);

  return [...changedItems, ...unchangedItems];
});

function updateBooleanItem (item: Extract<ConfigStoreItem, { type: 'boolean' }>, value: boolean) {
  item.value = value;
  saveChangesToLocalStorage();
}

function undoChangesToItem (item: ConfigStoreItem) {
  item.value = item.default;

  saveChangesToLocalStorage();
}

const changesStringOnPageLoad = ref('[]');
const changesString = computed(() => {
  const changes = configStore.items
    .filter(item => item.value !== item.default)
    .map(item => ({
      name: item.name,
      value: item.value
    }));
  return JSON.stringify(changes);
});

function saveChangesToLocalStorage () {
  localStorage.setItem('configStoreChanges', changesString.value);
}

const showReloadPrompt = computed(() => {
  return changesString.value !== changesStringOnPageLoad.value;
});

function reload () {
  window.location.reload();
}

onMounted(() => {
  isPopoverOpen.value = configStore.pinPopover;

  const changesString = localStorage.getItem('configStoreChanges');
  if (!changesString) {
    return;
  }
  try {
    const changes = (JSON.parse(changesString) as Pick<ConfigStoreItem, 'name' | 'value'>[])
      .filter(change => configStore.items.some(i => i.name === change.name));
    changesStringOnPageLoad.value = changesString;
    for (const change of changes) {
      const item = configStore.items.find(i => i.name === change.name);
      if (item) {
        item.value = change.value;
      }
    }
  } catch (error) {
    console.error('Failed to parse saved config store changes from localStorage on page load:', error);
    changesStringOnPageLoad.value = '[]';
  }
});
</script>
