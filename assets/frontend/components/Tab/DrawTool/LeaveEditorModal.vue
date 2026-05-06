<template>
  <ModalGeneric
    v-model:open="isModalOpen"
    data-id="modal-draw-tool-exit-confirm"
    title="Leave editor?"
    description="You have unsaved changes. Do you want to save them?"
    show-close-button
    no-actions
  >
    <template #actions>
      <div class="mt-8 flex flex-wrap justify-end gap-2">
        <UButton
          label="Discard changes"
          color="neutral"
          variant="ghost"
          :disabled="es.isLeavingEditor"
          @click="void es.discardAndLeaveEditor()"
        />
        <UButton
          label="Save and leave"
          color="neutral"
          :loading="es.isLeavingEditor"
          @click="void es.saveAndLeaveEditor()"
        />
      </div>
    </template>
  </ModalGeneric>
</template>

<script setup lang="ts">
const es = useDrawToolEditorStore();

const isModalOpen = computed({
  get: () => es.showLeaveEditorModal,
  set: open => {
    if (!open) {
      es.cancelLeaveEditorRequest();
      return;
    }

    es.showLeaveEditorModal = true;
  }
});
</script>
