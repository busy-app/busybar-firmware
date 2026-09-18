<template>
  <ModalGeneric
    v-model:open="open"
    data-id="modal-uploader-app"
    :title="confirming ? stageResult!.staged.name : 'Add your file'"
    wide
    :dismissible="!uploading && !installing"
    :show-close-button="!file || failed"
    :description="file && !uploading && !failed && !confirming ? 'This file will be added to your BUSY Bar and the local web interface.' : undefined"
  >
    <template
      v-if="confirming && stagedIcon"
      #icon
    >
      <img
        :src="stagedIcon"
        alt=""
        width="40"
        height="40"
        class="size-10 shrink-0 [image-rendering:pixelated]"
      >
    </template>

    <template #body>
      <UFileUpload
        v-if="!file"
        v-model="file"
        data-id="modal-uploader-app-file-upload"
        accept=".tgz"
        class="h-[396px] w-full"
        label="Add app file (.tgz)"
        description="Drag and drop to upload"
        :ui="{
          base: 'bg-transparent border-solid rounded-2xl cursor-pointer transition-colors data-[dragging=true]:bg-transparent data-[dragging=true]:border-accented',
          label: 'text-lg mt-4',
          description: 'text-sm mt-1',
          actions: 'mt-6',
        }"
      >
        <template #leading>
          <div class="flex size-12 items-center justify-center rounded-full bg-accented/25">
            <UIcon
              name="i-bi-upload"
              class="size-6"
            />
          </div>
        </template>

        <template #actions>
          <UButton
            data-id="modal-uploader-app-select-file-button"
            label="Select file"
            color="neutral"
          />
        </template>
      </UFileUpload>

      <div
        v-else-if="failed"
        data-id="modal-uploader-app-error"
        class="flex flex-col gap-6"
      >
        <div class="flex items-start gap-2">
          <UIcon
            name="i-bi-error-fill"
            class="size-6 shrink-0 text-error"
          />

          <div
            v-if="appMismatch"
            data-id="modal-uploader-app-error-mismatch"
            class="min-w-0 flex-1"
          >
            This file contains a different app. Select an update package for this app.
          </div>
          <div
            v-else
            class="min-w-0 flex-1"
          >
            Unable to upload this file. Try these steps until the issue is resolved:
          </div>
        </div>

        <div
          v-if="!appMismatch"
          class="rounded-xl bg-accented/25 p-4"
        >
          <ul class="list-disc space-y-5 ps-5 text-sm text-highlighted/90">
            <li>Check the file format — only .tgz files are supported</li>
            <li>Check the API version — the app must use a supported version</li>
            <li>Try again later — the server may be unavailable right now</li>
          </ul>
        </div>
      </div>

      <div
        v-else-if="uploading"
        data-id="modal-uploader-app-progress"
        class="flex flex-col gap-6 pt-4"
      >
        <div class="flex items-start gap-2">
          <UIcon
            name="i-bi-upload"
            class="size-6 shrink-0"
          />

          <div class="min-w-0 flex-1">
            {{ uploaded ? 'Checking the app. Do not disconnect your BUSY Bar.' : 'Uploading the file. Do not disconnect your BUSY Bar.' }}
          </div>

          <div
            v-if="!uploaded"
            data-id="modal-uploader-app-progress-value"
            class="text-muted"
          >
            {{ progress }}%
          </div>
        </div>

        <UProgress
          :model-value="uploaded ? null : progress"
          color="success"
          size="lg"
        />
      </div>

      <TabAppsAppInfo
        v-else-if="confirming"
        :app="stageResult!.staged"
        :installed="stageResult!.installed"
      />

      <div
        v-else
        data-id="modal-uploader-app-file-selected"
        class="flex items-center gap-2 rounded-xl border border-accented p-3"
      >
        <div class="flex items-center p-2">
          <UIcon
            name="i-bi-archive"
            class="size-6"
          />
        </div>

        <div
          data-id="modal-uploader-app-file-name"
          class="min-w-0 flex-1 truncate font-medium text-toned"
        >
          {{ file.name }}
        </div>

        <UButton
          data-id="modal-uploader-app-remove-file-button"
          icon="i-bi-trash"
          color="neutral"
          variant="soft"
          square
          :ui="{
            base: 'p-2.5 rounded-full bg-accented/50'
          }"
          @click="removeFile"
        />
      </div>
    </template>

    <template #actions>
      <div
        v-if="appMismatch"
        class="mt-2 flex justify-end gap-2"
      >
        <UButton
          data-id="modal-uploader-app-error-cancel-button"
          label="Cancel"
          color="neutral"
          variant="ghost"
          size="lg"
          class="min-w-20 justify-center"
          @click="() => { open = false; }"
        />

        <UButton
          data-id="modal-uploader-app-error-back-button"
          label="Back to file selection"
          color="neutral"
          size="lg"
          class="min-w-20 justify-center"
          @click="resetToStart"
        />
      </div>

      <div
        v-else-if="file && !failed"
        class="mt-2 flex justify-end gap-2"
      >
        <UButton
          data-id="modal-uploader-app-cancel-button"
          label="Cancel"
          color="neutral"
          variant="ghost"
          size="lg"
          class="min-w-20 justify-center"
          :disabled="installing"
          @click="cancel"
        />

        <UButton
          v-if="confirming"
          data-id="modal-uploader-app-confirm-button"
          label="Confirm"
          color="neutral"
          size="lg"
          class="min-w-20 justify-center"
          :loading="installing"
          @click="installFile"
        />

        <UButton
          v-else-if="!uploading"
          data-id="modal-uploader-app-add-button"
          label="Add"
          color="neutral"
          size="lg"
          class="min-w-20 justify-center"
          @click="stageFile"
        />
      </div>
    </template>
  </ModalGeneric>
</template>

<script setup lang="ts">
import type { AppInfo, AppStageResult } from '@busy-app/busy-lib';

const props = defineProps<{
  updateAppId?: string;
}>();

const emit = defineEmits<{
  (e: 'installed', app: AppInfo, updated: boolean): void;
}>();

const open = defineModel<boolean>('open', { default: false });

const APP_FILE_EXTENSION = '.tgz';
const APP_FILE_MAX_SIZE = 4 * 1024 * 1024;

let uploadController: AbortController | null = null;

const toast = useToast();
const appsStore = useAppsStore();

const file = ref<File | null>(null);

const stageResult = ref<AppStageResult>();
const stagedIcon = ref<string>();
const uploading = ref(false);
const progress = ref(0);
const installing = ref(false);
const failed = ref(false);
const appMismatch = ref(false);

const confirming = computed(() => !!stageResult.value && !uploading.value && !failed.value);
const uploaded = computed(() => progress.value >= 100);

function removeFile () {
  file.value = null;
  stageResult.value = undefined;
}

function resetToStart () {
  file.value = null;
  stageResult.value = undefined;
  stagedIcon.value = undefined;
  progress.value = 0;
  failed.value = false;
  appMismatch.value = false;
}

async function stageFile () {
  if (!file.value) {
    return;
  }

  uploadController = new AbortController();
  uploading.value = true;
  progress.value = 0;
  failed.value = false;

  try {
    const result = await appsStore.stageApp(file.value, uploadController.signal, value => {
      progress.value = value;
    });

    if (props.updateAppId && result.staged.id !== props.updateAppId) {
      appMismatch.value = true;
      failed.value = true;
      return;
    }

    stageResult.value = result;
    stagedIcon.value = await appsStore.readIcon(result.staged.icon_path);
  } catch (error) {
    if (!uploadController.signal.aborted) {
      console.error('App staging failed', error);
      failed.value = true;
    }
  } finally {
    uploading.value = false;
    uploadController = null;
  }
}

async function installFile () {
  if (!stageResult.value) {
    return;
  }

  installing.value = true;

  try {
    await appsStore.installApp(stageResult.value.install_key);

    emit('installed', stageResult.value.staged, !!stageResult.value.installed);
    open.value = false;
  } catch (error) {
    console.error('App installation failed', error);
    failed.value = true;
  } finally {
    installing.value = false;
  }
}

function cancel () {
  if (uploading.value) {
    uploadController?.abort();
    return;
  }

  open.value = false;
}

watch(file, value => {
  if (!value) {
    return;
  }

  if (!value.name.toLowerCase().endsWith(APP_FILE_EXTENSION)) {
    rejectFile('Unsupported file', `Only ${APP_FILE_EXTENSION} app packages can be uploaded.`);
  } else if (value.size > APP_FILE_MAX_SIZE) {
    rejectFile('File is too large', `App packages can be up to ${bytesToSize(APP_FILE_MAX_SIZE)}.`);
  }
});

function rejectFile (title: string, description: string) {
  toast.add({
    title,
    description,
    icon: 'i-bi-alert',
    color: 'error'
  });

  file.value = null;
}

watch(open, value => {
  if (value) {
    resetToStart();
  }
});
</script>
