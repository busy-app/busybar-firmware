<template>
  <ModalGeneric
    v-model:open="pms.showRemovePasswordModal"
    data-id="modal-remove-password"
    :dismissible="false"
    title="Remove password"
    description="If the password is not set, anyone on the same Wi-Fi network will be able to access the device via this page."
    wide
    :primary-action-props="{
      label: 'Remove password',
      loading: pms.loading,
      disabled: isInvalid,
      onClick: pms.removePassword
    }"
    :secondary-action-props="{
      label: 'Cancel',
      variant: 'ghost',
      disabled: pms.loading,
      onClick: () => { pms.showRemovePasswordModal = false; }
    }"
  >
    <template #body>
      <UFormField
        v-if="apiStore.apiKey"
        label="Current password"
        :error="pms.currentPasswordValidation"
      >
        <UInput
          v-model="pms.passwordModel.current"
          v-maska="'##########'"
          name="current-password"
          size="xl"
          variant="soft"
          :ui="{ base: 'ring-1 ring-glass bg-accented/50' }"
          :type="pms.passwordModel.showCurrent ? 'text' : 'password'"
          placeholder="Enter password"
          @update:model-value="pms.passwordModel.currentWrong = false"
          @keyup.enter="isInvalid || pms.loading ? null : pms.removePassword()"
        >
          <template #trailing>
            <UButton
              :icon="pms.passwordModel.showCurrent ? 'i-bi-eye' : 'i-bi-eye-shut'"
              variant="ghost"
              color="neutral"
              square
              class="rounded-full"
              :ui="{
                leadingIcon: 'size-6 text-muted'
              }"
              @click="() => { pms.passwordModel.showCurrent = !pms.passwordModel.showCurrent; }"
            />
          </template>
        </UInput>
      </UFormField>
    </template>
  </ModalGeneric>
</template>

<script lang="ts" setup>
import { vMaska } from 'maska/vue';

const pms = usePasswordModalStore();
const apiStore = useApiStore();

const isInvalid = computed(() => pms.currentPasswordValidation !== undefined || (!!apiStore.apiKey && pms.passwordModel.current === ''));
</script>
