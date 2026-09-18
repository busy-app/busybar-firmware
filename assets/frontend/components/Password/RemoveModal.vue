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
      <form
        v-if="apiStore.apiKey"
        @submit.prevent="isInvalid || pms.loading ? null : pms.removePassword()"
      >
        <input
          value="BUSY Bar"
          type="text"
          name="username"
          autocomplete="username"
          class="sr-only"
          tabindex="-1"
          aria-hidden="true"
          readonly
        >

        <UFormField
          label="Current password"
          :error="pms.currentPasswordValidation"
        >
          <UInput
            v-model="pms.passwordModel.current"
            v-maska="'##########'"
            name="current-password"
            autocomplete="current-password"
            inputmode="numeric"
            size="xl"
            variant="soft"
            :ui="{ base: 'ring-1 ring-glass bg-accented/50' }"
            :type="pms.passwordModel.showCurrent ? 'text' : 'password'"
            placeholder="Enter password"
            @update:model-value="pms.passwordModel.currentWrong = false"
          >
            <template #trailing>
              <UButton
                :icon="pms.passwordModel.showCurrent ? 'i-bi-eye' : 'i-bi-eye-shut'"
                type="button"
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

        <button
          type="submit"
          class="hidden"
          tabindex="-1"
          aria-hidden="true"
        />
      </form>
    </template>
  </ModalGeneric>
</template>

<script lang="ts" setup>
import { vMaska } from 'maska/vue';

const pms = usePasswordModalStore();
const apiStore = useApiStore();

const isInvalid = computed(() => pms.currentPasswordValidation !== undefined || (!!apiStore.apiKey && pms.passwordModel.current === ''));
</script>
