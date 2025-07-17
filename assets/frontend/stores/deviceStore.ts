import { defineStore } from 'pinia';

export interface deviceVersion {
  branch: string;
  version: string;
  build_date: string;
  commit_hash: string;
}

export type UpdateStage = 'idle' | 'uploading' | 'unpacking' | 'updating' | 'error';

export const useDeviceStore = defineStore('device', () => {
  const barUrl = useRuntimeConfig().public.barUrl;
  const toast = useToast();

  const version = ref<deviceVersion>({
    branch: '',
    version: '',
    build_date: '',
    commit_hash: ''
  });

  async function updateDeviceVersion (throwError: boolean = false) {
    const deviceVersion = await $fetch<deviceVersion>(`${barUrl}/api/v0/version`, { timeout: 3000 })
      .then(response => {
        if (!response || typeof response !== 'object') {
          throw new Error('Empty response');
        }
        console.log('Device version fetched:', response);
        return response;
      })
      .catch(error => {
        if (throwError) {
          throw error;
        }
        console.error('Error fetching device version:', error);
        toast.add({
          id: 'device-version-error',
          title: 'Failed to fetch device version',
          description: error.message || 'Unknown error. Check your connection and try again.',
          icon: 'i-tabler-alert-triangle-filled',
          color: 'error',
          duration: 10000
        });
        return version.value;
      });
    version.value = deviceVersion;
  }

  // Firmware update
  const update = ref({
    firmwareBundleName: 'firmware',
    firmwareFile: null as File | null,
    stage: 'idle' as UpdateStage,
    progress: 0,
    error: null as string | null
  });

  async function uploadFirmware () {
    const xhr = new XMLHttpRequest();
    xhr.open('POST', `${barUrl}/api/v0/update?name=${update.value.firmwareBundleName}`);
    xhr.setRequestHeader('Content-Type', 'application/octet-stream');

    xhr.upload.onprogress = event => {
      if (event.lengthComputable) {
        update.value.progress = Math.round((event.loaded / event.total) * 100);

        if (update.value.progress === 100) {
          update.value.stage = 'unpacking';
        }
      } else {
        console.log(`Uploaded ${event.loaded} bytes`);
      }
    };

    xhr.onload = () => {
      if (xhr.status >= 200 && xhr.status < 400) {
        update.value.stage = 'updating';
        toast.add({
          title: 'Update initiated',
          description: 'The device will reboot to apply the update. Pay attention to the front screen.',
          icon: 'i-tabler-check',
          color: 'success',
          duration: 10000
        });
      } else {
        console.error('Upload failed:', xhr.status, xhr.responseText);
        update.value.stage = 'error';
        toast.add({
          title: 'Update failed',
          description: `Error ${xhr.status}: ${xhr.responseText}`,
          icon: 'i-tabler-alert-triangle-filled',
          color: 'error',
          duration: 10000
        });
        update.value.error = `Error ${xhr.status}: ${xhr.responseText}`;
      }
    };

    update.value.stage = 'uploading' as UpdateStage;
    update.value.progress = 0;
    xhr.send(update.value.firmwareFile);

    await new Promise<void>(resolve => {
      xhr.onloadend = () => {
        resolve();
      };
    });

    update.value.firmwareFile = null;
    if (update.value.stage !== 'error') {
      update.value.stage = 'updating' as UpdateStage;
      update.value.progress = 0;
    }
  }

  return {
    version,
    updateDeviceVersion,
    update,
    uploadFirmware
  };
});
