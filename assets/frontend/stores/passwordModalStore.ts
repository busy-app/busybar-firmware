import { defineStore } from 'pinia';

export const usePasswordModalStore = defineStore('passwordModal', () => {
  const loading = ref(false);

  const showSetPasswordModal = ref(false);
  const showUpdatePasswordModal = ref(false);
  const showRemovePasswordModal = ref(false);

  const passwordModel = ref({
    current: '',
    showCurrent: false,
    currentWrong: false,
    new: '',
    showNew: false
  });
  const newPasswordValidation = computed(() => {
    return (
      passwordModel.value.new !== ''
      && (
        /[^0-9]/.test(passwordModel.value.new)
          ? 'Invalid password (only digits allowed)'
          : passwordModel.value.new.length > 10
            ? 'Password too long'
            : passwordModel.value.new.length < 4 && passwordModel.value.new !== ''
              ? 'Password too short'
              : undefined
      ) as string | undefined
    );
  });
  const currentPasswordValidation = computed(() => {
    const key = useApiStore().apiKey;
    if (key && passwordModel.value.current !== key) {
      return 'Incorrect password. Try again.' as string;
    }
    if (passwordModel.value.currentWrong) {
      return 'Incorrect password. Try again.' as string;
    }
    return undefined;
  });

  async function setPassword () {
    const deviceStore = useDeviceStore();
    const httpApiAccess = ref(await deviceStore.getHttpAPIAccess());
    loading.value = true;
    await deviceStore.setHttpAPIAccess('key', passwordModel.value.new);
    deviceStore.httpAPIAccess = await deviceStore.fetchHttpAPIAccess();
    httpApiAccess.value = deviceStore.httpAPIAccess;
    loading.value = false;
    showSetPasswordModal.value = false;
    showUpdatePasswordModal.value = false;
  }

  async function removePassword () {
    const deviceStore = useDeviceStore();
    const httpApiAccess = ref(await deviceStore.getHttpAPIAccess());
    loading.value = true;
    await deviceStore.setHttpAPIAccess('enabled');
    deviceStore.httpAPIAccess = await deviceStore.fetchHttpAPIAccess();
    httpApiAccess.value = deviceStore.httpAPIAccess;
    loading.value = false;
    showRemovePasswordModal.value = false;
  }

  return {
    showSetPasswordModal,
    showUpdatePasswordModal,
    showRemovePasswordModal,

    passwordModel,
    newPasswordValidation,
    currentPasswordValidation,

    loading,
    setPassword,
    removePassword
  };
});
