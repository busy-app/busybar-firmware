// eslint-disable-next-line @typescript-eslint/no-explicit-any
export async function handleHTTPError (error: any, title: string, shouldCheckForConnection?: boolean, duration?: number) {
  const deviceStore = useDeviceStore();
  const configStore = useConfigStore();

  if (error?.status === 403) {
    await navigateTo('/login');
    return;
  }

  console.error(title, error);

  if (shouldCheckForConnection) {
    console.debug('[HTTP error handler]: checking device connection...');
    await deviceStore.checkConnection();
  }

  if (!deviceStore.isConnected) {
    return;
  }

  toast.add({
    id: 'device-status-error',
    title,
    description: parseError(error),
    icon: 'i-bi-alert',
    color: 'error',
    duration: typeof duration === 'number' ? duration : Number(configStore.get('notificationDuration'))
  });
}

// eslint-disable-next-line @typescript-eslint/no-explicit-any
export function isDisplayPriorityConflict (error: any) {
  return error?.status === 409;
}

export function notifyDisplayPriorityConflict () {
  const configStore = useConfigStore();

  toast.add({
    id: 'device-status-priority-conflict',
    title: 'Can\'t show status',
    description: 'Another activity is already running on your BUSY Bar. End it to show a custom status.',
    icon: 'i-bi-alert',
    color: 'warning',
    duration: Number(configStore.get('notificationDuration'))
  });
}

const genericErrorMessage = 'Unknown error. Check your connection and try again.';
// eslint-disable-next-line @typescript-eslint/no-explicit-any
function parseError (error: any) {
  if (error?.data?.error) {
    return error.data.error;
  }
  if (String(error).length) {
    const s = String(error);
    if (s.includes('Error:')) {
      const index = s.indexOf('Error:');
      const sliced = s.slice(index + 6);
      return sliced.trim();
    }
    return s;
  }

  return genericErrorMessage;
}
