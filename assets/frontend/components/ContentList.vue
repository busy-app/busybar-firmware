<template>
  <div class="grid grid-cols-2 gap-2">
    <ul
      v-for="list in props.items"
      :key="list[0]?.title"
      class="flex flex-col gap-3"
    >
      <template
        v-for="item in list"
        :key="item.title"
      >
        <li class="flex break-inside-avoid flex-col gap-0.5 sm:flex-row sm:gap-2">
          <div class="flex-2 text-muted">{{ item.title }}</div>
          <template v-if="item.loading">
            <USkeleton class="h-4 flex-3 rounded-full" />
          </template>
          <template v-else>
            <div
              class="flex-3 truncate"
              :class="item.class"
            >{{ item.value || 'Unknown' }}</div>
          </template>
        </li>
      </template>
    </ul>
  </div>
</template>

<script setup lang="ts">
const props = defineProps<{
  items: {
    title: string;
    value: string | undefined;
    loading?: boolean;
    class?: string;
  }[][];
}>();
</script>
