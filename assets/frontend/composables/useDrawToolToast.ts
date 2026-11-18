const DRAW_TOOL_RESULT_TOAST_ID = 'draw-tool-result';
const FORBIDDEN_STATUS = 403;

function isForbiddenError (error: unknown) {
  return !!error && typeof error === 'object' && 'status' in error && error.status === FORBIDDEN_STATUS;
}

export function showDrawToolSuccessToast (title: string) {
  toast.add({
    id: DRAW_TOOL_RESULT_TOAST_ID,
    title,
    icon: 'i-bi-checkmark-circle-fill',
    color: 'success'
  });
}

export async function showDrawToolErrorToast (title: string, error: unknown) {
  console.error(title, error);

  if (isForbiddenError(error)) {
    await navigateTo('/login');
    return;
  }

  toast.add({
    id: DRAW_TOOL_RESULT_TOAST_ID,
    title,
    icon: 'i-bi-alert',
    color: 'error'
  });
}
