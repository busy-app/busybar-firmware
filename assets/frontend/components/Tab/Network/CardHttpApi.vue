<template>
  <SectionCard
    v-if="deviceStore.httpAPIAccess"
    data-id="network-section-http-api"
    title="HTTP API"
    icon="i-bi-http-api"
  >
    <div class="flex justify-between items-center">
      <div>Over USB</div>
      <UButton
        variant="link"
        class="p-0 gap-0.5"
        href="http://10.0.4.20/docs"
        target="_blank"
      >
        <span class="underline">http://10.0.4.20/docs</span>
        <UIcon
          name="i-bi-open-in-new"
          class="size-4"
        />
      </UButton>
    </div>
    <div class="flex flex-col gap-1">
      <div class="flex justify-between items-center gap-y-4 gap-x-6">
        <div>Over Wi-Fi</div>
        <UButton
          v-if="connected && deviceStore.httpAPIAccess.mode !== 'disabled'"
          variant="link"
          class="p-0 gap-0.5"
          :href="`http://${wifiStore.wifi?.ip_config?.address}/docs`"
          target="_blank"
        >
          <span class="underline">http://{{ wifiStore.wifi?.ip_config?.address }}/docs</span>
          <UIcon
            name="i-bi-open-in-new"
            class="size-4"
          />
        </UButton>
      </div>
      <div
        v-if="!connected"
        class="text-sm text-muted"
      >
        Connect to a Wi-Fi network to enable access to HTTP API over Wi-Fi
      </div>
      <template v-else>
        <div class="flex justify-between items-center gap-y-4 gap-x-6 mt-3">
          <div class="flex flex-col gap-1">
            <div>HTTP API access</div>
            <div class="text-sm text-muted">Send requests and control BUSY Bar when connected via Wi-Fi</div>
          </div>

          <USwitch
            v-model="httpApiSwitchModel"
            data-id="network-section-http-api-switch"
            :loading="loading.access"
            @update:model-value="handleHttpApiToggle"
          />
        </div>

        <div
          v-if="deviceStore.httpAPIAccess.mode !== 'disabled'"
          class="flex justify-between items-center gap-y-4 gap-x-6 mt-3"
        >
          <div class="flex flex-col gap-1">
            <div class="flex items-center gap-2">
              <div>Password protection</div>
              <UBadge
                v-if="deviceStore.httpAPIAccess.mode === 'key'"
                data-id="network-section-http-api-status-secure"
                icon="i-bi-lock-simple-fill"
                label="Secure"
                color="success"
                size="sm"
                class="rounded-full"
              />
              <UBadge
                v-else-if="deviceStore.httpAPIAccess.mode === 'enabled'"
                data-id="network-section-http-api-status-insecure"
                icon="i-bi-alert-fill"
                label="Not set"
                color="warning"
                size="sm"
                class="rounded-full"
              />
            </div>
            <div
              v-if="deviceStore.httpAPIAccess.mode === 'key'"
              class="text-sm text-muted"
            >
              A password will be asked to access the BUSY Bar web interface via Wi-Fi
            </div>
            <div
              v-else-if="deviceStore.httpAPIAccess.mode === 'enabled'"
              class="text-sm text-muted"
            >
              Without a password anyone on your Wi-Fi can access your BUSY Bar
            </div>
          </div>

          <UDropdownMenu
            v-if="deviceStore.httpAPIAccess.mode === 'key'"
            data-id="network-section-http-api-security-dropdown"
            :items="[
              {
                label: 'Change',
                icon: 'i-bi-edit',
                onSelect: () => {
                  pms.passwordModel.current = '';
                  pms.passwordModel.currentWrong = false;
                  pms.passwordModel.new = '';
                  pms.showUpdatePasswordModal = true;
                }
              },
              {
                label: 'Remove',
                icon: 'i-bi-unlock',
                onSelect: () => {
                  pms.passwordModel.current = '';
                  pms.passwordModel.currentWrong = false;
                  pms.showRemovePasswordModal = true;
                }
              }
            ]"
            :content="{
              align: 'end',
              side: 'bottom',
              sideOffset: 8
            }"
            :ui="{
            }"
          >
            <UButton
              icon="i-bi-more"
              variant="ghost"
              color="neutral"
              square
            />
          </UDropdownMenu>

          <UButton
            v-else
            data-id="network-section-http-api-set-password-button"
            label="Set password"
            variant="soft"
            color="primary"
            @click="() => {
              pms.passwordModel.current = '';
              pms.passwordModel.currentWrong = false;
              pms.passwordModel.new = '';
              pms.showSetPasswordModal = true;
            }"
          />
        </div>
      </template>

      <ModalGeneric
        v-model:open="showEnableHttpApiModal"
        data-id="modal-http-api"
        title="Enable access to HTTP API?"
        description="Anyone on the same Wi-Fi network will be able to access the device via this page. We recommend setting a password that will be asked each time you open this page with a BUSY Bar connected via Wi-Fi."
        show-close-button
        wide
        :primary-action-props="{
          label: 'Set password and enable',
          loading: loading.access,
          onClick: async () => {
            showEnableHttpApiModal = false;
            pms.passwordModel.current = '';
            pms.passwordModel.currentWrong = false;
            pms.passwordModel.new = '';
            pms.showSetPasswordModal = true;
          }
        }"
        :secondary-action-props="{
          label: 'Continue without password',
          loading: loading.access,
          onClick: async () => {
            loading.access = true;
            await deviceStore.setHttpAPIAccess('enabled');
            loading.access = false;
            showEnableHttpApiModal = false;
          }
        }"
      />
    </div>
  </SectionCard>
</template>

<script setup lang="ts">
const deviceStore = useDeviceStore();
const wifiStore = useWifiStore();
const pms = usePasswordModalStore();

const loading = ref({
  access: false
});

const httpApiSwitchModel = ref(false);
const showEnableHttpApiModal = ref(false);

const connected = computed(() => wifiStore.wifi?.state === 'connected');

watch(() => deviceStore.httpAPIAccess, access => {
  if (!access) {
    return;
  }
  if (access.mode === 'key' || access.mode === 'enabled') {
    httpApiSwitchModel.value = true;
  } else {
    httpApiSwitchModel.value = false;
  }
}, { immediate: true, deep: true });

async function handleHttpApiToggle (value: boolean) {
  if (!value) {
    loading.value.access = true;
    await deviceStore.setHttpAPIAccess('disabled');
    loading.value.access = false;
  } else {
    if (deviceStore.httpAPIAccess?.mode === 'key') {
      httpApiSwitchModel.value = true;
    } else {
      showEnableHttpApiModal.value = true;
      httpApiSwitchModel.value = false;
    }
  }
}
</script>
