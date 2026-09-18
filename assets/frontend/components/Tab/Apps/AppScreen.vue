<template>
  <SectionCard
    :data-id="`apps-section-${appId}`"
    :title="title"
    :ui="{ title: 'font-medium', titleWrapper: 'gap-2' }"
  >
    <template #leading-actions>
      <SectionBackButton
        :data-id="`apps-section-${appId}-back-button`"
        @click="emit('back')"
      />
    </template>

    <template #actions>
      <UButton
        v-if="updatable"
        :data-id="`apps-section-${appId}-update-button`"
        label="Update app"
        icon="i-bi-upload"
        color="neutral"
        variant="outline"
        class="justify-center"
        :ui="{
          base: 'px-4 py-2.5 gap-1.5'
        }"
        @click="emit('update')"
      />

      <UButton
        :data-id="`apps-section-${appId}-start-button`"
        label="Start"
        icon="i-bi-play-fill"
        color="neutral"
        variant="solid"
        class="justify-center"
        :ui="{
          base: 'px-4 py-2.5 gap-1.5'
        }"
        @click="startApp"
      />
    </template>

    <template #raw-body>
      <slot />
    </template>
  </SectionCard>
</template>

<script setup lang="ts">
defineProps<{
  appId: string;
  title: string;
  updatable?: boolean;
}>();

const emit = defineEmits<{
  (e: 'back' | 'update'): void;
}>();

const toast = useToast();

function startApp () {
  toast.add({
    title: 'Coming soon',
    description: 'Starting apps from the web interface is not available yet.',
    icon: 'i-bi-info'
  });
}
</script>
