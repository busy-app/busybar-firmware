<template>
  <ModalGeneric
    v-model:open="pms.showSetPasswordModal"
    data-id="modal-set-password"
    :dismissible="false"
    title="Set password"
    description="This password will be asked each time you open this page with a BUSY Bar connected via Wi-Fi. Remember your password, as a forgotten one cannot be recovered, but only reset via a wired connection."
    :primary-action-props="{
      label: 'Set password',
      loading: pms.loading,
      disabled: isInvalid,
      onClick: pms.setPassword
    }"
    :secondary-action-props="{
      label: 'Cancel',
      variant: 'ghost',
      disabled: pms.loading,
      onClick: () => { pms.showSetPasswordModal = false; }
    }"
  >
    <template #icon>
      <UIcon
        name="i-bi-password"
        class="size-8 text-muted"
      />
    </template>
    <template #body>
      <form @submit.prevent="isInvalid || pms.loading ? null : pms.setPassword()">
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
          label="Password"
          :error="pms.newPasswordValidation"
        >
          <UInput
            v-model="pms.passwordModel.new"
            v-maska="'##########'"
            name="new-password"
            autocomplete="new-password"
            passwordrules="required: digit; allowed: digit; minlength: 4; maxlength: 10;"
            inputmode="numeric"
            size="xl"
            variant="soft"
            :ui="{ base: 'ring-1 ring-glass bg-accented/50' }"
            :type="pms.passwordModel.showNew ? 'text' : 'password'"
            placeholder="From 4 to 10 digits"
          >
            <template #trailing>
              <UButton
                :icon="pms.passwordModel.showNew ? 'i-bi-eye' : 'i-bi-eye-shut'"
                type="button"
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

const isInvalid = computed(() => pms.newPasswordValidation !== undefined || pms.passwordModel.new === '');
</script>
