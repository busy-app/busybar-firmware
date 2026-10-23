<template>
  <SectionCard
    data-id="settings-section-home-assistant"
    title="Home Assistant"
  >
    <template #leading-actions>
      <SectionBackButton
        data-id="settings-section-home-assistant-back-button"
        @click="emit('back')"
      />
    </template>

    <template #actions>
      <UButton
        variant="link"
        class="p-0 gap-0.5 font-normal whitespace-nowrap"
        :href="appConfig.links.homeAssistantDocs"
        target="_blank"
      >
        <span class="underline">How to connect</span>
        <UIcon
          name="i-bi-open-in-new"
          class="size-4"
        />
      </UButton>
    </template>

    <template #raw-body>
      <div
        data-id="settings-section-home-assistant-tokens"
        class="flex flex-col gap-6"
      >
        <div
          v-for="token in accessTokensStore.tokens"
          :key="token.short_id"
          :data-id="`home-assistant-token-${token.short_id}`"
          class="flex flex-col gap-1 rounded-group"
        >
          <div class="bg-accented/25 dark:bg-elevated/75 p-4 flex items-center gap-6">
            <div class="grow min-w-0 flex flex-col gap-1">
              <div class="truncate">{{ token.name }}</div>
              <div class="text-sm text-muted truncate">{{ token.display_id }}</div>
            </div>
            <UButton
              icon="i-bi-trash-fill"
              label="Delete token"
              variant="soft"
              color="neutral"
              class="rounded-full shrink-0 bg-accented/50 text-toned"
              @click="() => { tokenToDelete = token; }"
            />
          </div>
          <div class="bg-accented/25 dark:bg-elevated/75 p-4">
            <ContentList
              :items="[
                [{ title: 'Date created', value: token.created_at ? formatDateDMY(Number(token.created_at)) : undefined }],
                [{ title: 'Last used', value: Number(token.last_used_at) ? formatTimeAgo(Number(token.last_used_at), now) : 'Never' }]
              ]"
            />
          </div>
        </div>
      </div>
    </template>
  </SectionCard>

  <ModalGeneric
    v-model:open="showTokenDeleteModal"
    data-id="modal-home-assistant-token-delete"
    title="Delete this token?"
    description="This Home Assistant token will be deleted. Apps and services that rely on it will lose access."
    :primary-action-props="{
      label: 'Delete token',
      variant: 'soft',
      color: 'error',
      loading: deletingToken,
      onClick: deleteToken
    }"
    :secondary-action-props="{
      label: 'Cancel',
      variant: 'ghost',
      color: 'neutral',
      disabled: deletingToken,
      onClick: () => { tokenToDelete = null }
    }"
  />
</template>

<script setup lang="ts">
import type { AccessToken } from '@busy-app/busy-lib';

const emit = defineEmits<{
  back: [];
}>();

let nowInterval: ReturnType<typeof setInterval> | null = null;

const accessTokensStore = useAccessTokensStore();
const appConfig = useAppConfig();

const now = ref(Date.now());
const tokenToDelete = ref<AccessToken | null>(null);
const deletingToken = ref(false);

const showTokenDeleteModal = computed({
  get: () => tokenToDelete.value !== null,
  set: value => {
    if (!value) {
      tokenToDelete.value = null;
    }
  }
});

async function deleteToken () {
  if (!tokenToDelete.value?.short_id) {
    return;
  }

  deletingToken.value = true;
  const deleted = await accessTokensStore.revokeToken(tokenToDelete.value.short_id);
  deletingToken.value = false;
  if (!deleted) {
    return;
  }

  tokenToDelete.value = null;

  if (accessTokensStore.tokens.length === 0) {
    emit('back');
  }
}

async function init () {
  await accessTokensStore.fetchTokens();
}

onMounted(async () => {
  window.scrollTo({ top: 0 });

  nowInterval = setInterval(() => {
    now.value = Date.now();
  }, 1000);

  window.addEventListener('device-reconnected', init);

  await init();
});

onBeforeUnmount(() => {
  window.removeEventListener('device-reconnected', init);

  if (nowInterval) {
    clearInterval(nowInterval);
  }
});
</script>
