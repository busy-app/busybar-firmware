// eslint-disable-next-line @typescript-eslint/no-explicit-any
export async function handleHTTPError (error: any, title: string, shouldCheckForConnection?: boolean) {
  const deviceStore = useDeviceStore();
  if (error.data?.error === 'Forbidden') {
    await navigateTo('/login');
    return undefined;
  }

  console.error(title, error);

  if (shouldCheckForConnection) {
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
    duration: 10000
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
