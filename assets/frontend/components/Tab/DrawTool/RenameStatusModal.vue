<template>
  <ModalGeneric
    v-model:open="open"
    data-id="modal-draw-tool-rename-status"
    title="Rename status"
    :primary-action-props="{
      label: 'Rename',
      disabled: !nameModel.trim(),
      loading: isRenaming,
      onClick: renameStatus
    }"
    :secondary-action-props="{
      label: 'Cancel',
      variant: 'ghost',
      disabled: isRenaming,
      onClick: () => { open = false; }
    }"
  >
    <template #icon>
      <UIcon
        name="i-bi-edit"
        class="size-8 text-muted"
      />
    </template>

    <template #body>
      <UInput
        v-model="nameModel"
        :maxlength="DRAW_TOOL_STATUS_NAME_MAX_LENGTH"
        name="status-name"
        size="xl"
        variant="soft"
        autofocus
        :ui="{ base: 'ring-1 ring-glass bg-accented/50' }"
        :disabled="isRenaming"
        @keyup.enter="renameStatus"
      >
        <template #trailing>
          <span
            class="text-xs text-muted tabular-nums"
            aria-live="polite"
            role="status"
          >
            {{ nameModel.length }}/{{ DRAW_TOOL_STATUS_NAME_MAX_LENGTH }}
          </span>
        </template>
      </UInput>
    </template>
  </ModalGeneric>
</template>

<script setup lang="ts">
const open = defineModel<boolean>('open', { default: false });

const es = useDrawToolEditorStore();
const dts = useDrawToolStore();

const nameModel = ref('');
const isRenaming = ref(false);

async function renameBusyBarTheme (statusName: string) {
  const themeName = createStatusApiName(statusName);

  if (!es.savedThemeName || themeName === es.savedThemeName) {
    return es.savedThemeName;
  }

  return dts.saveSavedStatusTheme(es.statusFileName, {
    themeName,
    previousThemeName: es.savedThemeName
  });
}

async function renameStatus () {
  const statusName = nameModel.value.trim();

  if (!statusName || isRenaming.value) {
    return;
  }

  isRenaming.value = true;

  try {
    const savedThemeName = await renameBusyBarTheme(statusName);

    es.renameStatus(statusName);

    if (savedThemeName) {
      es.markThemeSaved(savedThemeName);
    }

    open.value = false;
  } catch (error) {
    await showDrawToolErrorToast('Couldn’t rename the status on BUSY Bar. Check the connection and try again.', error);
  } finally {
    isRenaming.value = false;
  }
}

watch(open, isOpen => {
  if (isOpen) {
    nameModel.value = es.statusName;
  }
});
</script>
