<template>
  <ModalGeneric
    v-model:open="pms.showUpdatePasswordModal"
    data-id="modal-update-password"
    :dismissible="false"
    title="Change password"
    description="Enter current and new passwords. Remember your password, as a forgotten one cannot be recovered, but only reset via a wired connection."
    wide
    :primary-action-props="{
      label: 'Update password',
      loading: pms.loading,
      disabled: isInvalid,
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
          @keyup.enter="isInvalid || pms.loading ? null : pms.setPassword()"
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

      <UFormField
        label="New password"
        :error="pms.newPasswordValidation"
      >
        <UInput
          v-model="pms.passwordModel.new"
          v-maska="'##########'"
          name="new-password"
          size="xl"
          variant="soft"
          :ui="{ base: 'ring-1 ring-glass bg-accented/50' }"
          :type="pms.passwordModel.showNew ? 'text' : 'password'"
          placeholder="From 4 to 10 digits"
          @keyup.enter="isInvalid || pms.loading ? null : pms.setPassword()"
        >
          <template #trailing>
            <UButton
              :icon="pms.passwordModel.showNew ? 'i-bi-eye' : 'i-bi-eye-shut'"
              variant="ghost"
              color="neutral"
              square
              class="rounded-full"
              :ui="{
                leadingIcon: 'size-6 text-muted'
              }"
              @click="() => { pms.passwordModel.showNew = !pms.passwordModel.showNew; }"
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

const isInvalid = computed(() => pms.newPasswordValidation !== undefined
  || pms.currentPasswordValidation !== undefined
  || (!!apiStore.apiKey && pms.passwordModel.current === '')
  || pms.passwordModel.new === '');
</script>
