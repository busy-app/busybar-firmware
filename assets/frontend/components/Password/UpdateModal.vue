<template>
  <ModalGeneric
    v-model:open="pms.showUpdatePasswordModal"
    :dismissible="false"
    title="Change password"
    description="Enter current and new passwords. Remember your password, as a forgotten one cannot be recovered, but only reset via a wired connection."
    wide
    :primary-action-props="{
      label: 'Update password',
      loading: pms.loading,
      disabled: pms.newPasswordValidation !== undefined || pms.currentPasswordValidation !== undefined || pms.passwordModel.current === '' || pms.passwordModel.new === '',
      onClick: pms.setPassword
    }"
    :secondary-action-props="{
      label: 'Cancel',
      variant: 'ghost',
      disabled: pms.loading,
      onClick: () => { pms.showUpdatePasswordModal = false; }
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

      <UFormField
        label="New password"
        :error="pms.newPasswordValidation"
      >
        <UInput
          v-model="pms.passwordModel.new"
          v-maska="'##########'"
          size="xl"
          variant="soft"
          :type="pms.passwordModel.showNew ? 'text' : 'password'"
          placeholder="From 4 to 10 digits"
        >
          <template #trailing>
            <UButton
              :icon="pms.passwordModel.showNew ? 'i-ri-eye-close-line' : 'i-ri-eye-line'"
              variant="ghost"
              color="neutral"
              square
              class="rounded-full"
              :ui="{
                leadingIcon: 'size-6 text-muted'
              }"
              @click="pms.passwordModel.showNew = !pms.passwordModel.showNew"
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
