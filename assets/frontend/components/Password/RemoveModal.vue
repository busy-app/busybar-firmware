<template>
  <ModalGeneric
    v-model:open="pms.showRemovePasswordModal"
    :dismissible="false"
    title="Remove password"
    description="If the password is not set, anyone on the same Wi-Fi network will be able to access the device via this page."
    wide
    :primary-action-props="{
      label: 'Remove password',
      loading: pms.loading,
      disabled: pms.currentPasswordValidation !== undefined || pms.passwordModel.current === '',
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
        label="Current password"
        :error="pms.currentPasswordValidation"
      >
        <UInput
          v-model="pms.passwordModel.current"
          v-maska="'##########'"
          size="xl"
          variant="soft"
          :type="pms.passwordModel.showCurrent ? 'text' : 'password'"
          placeholder="Enter password"
          @update:model-value="pms.passwordModel.currentWrong = false"
        >
          <template #trailing>
            <UButton
              :icon="pms.passwordModel.showCurrent ? 'i-ri-eye-close-line' : 'i-ri-eye-line'"
              variant="ghost"
              color="neutral"
              square
              class="rounded-full"
              :ui="{
                leadingIcon: 'size-6 text-muted'
              }"
              @click="pms.passwordModel.showCurrent = !pms.passwordModel.showCurrent"
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
</script>
