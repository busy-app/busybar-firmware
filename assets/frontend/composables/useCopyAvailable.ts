export const copyAvailable = computed(() => !!(navigator.clipboard || document.queryCommandSupported('copy')));
