<template>
  <UButton
    class="px-0"
    @click="copyToClipboard"
  >
    {{ props.text }}
    <UTooltip
      :delay-duration="0"
      :text="copyState === 'idle' ? 'Copy to clipboard' : copyState === 'copied' ? 'Copied!' : undefined"
    >
      <UIcon
        :name="currentIcon"
        class="size-4"
        :class="iconClassName"
      />
    </UTooltip>
  </UButton>
</template>

<script setup lang="ts">
const props = defineProps<{
  text: string;
}>();

const toast = useToast();

const copyState = ref<'idle' | 'copying' | 'copied' | 'error'>('idle');

const currentIcon = computed(() => {
  switch (copyState.value) {
    case 'idle':
      return 'i-bi-copy';
    case 'copied':
      return 'i-bi-checkmark';
    case 'error':
      return 'i-bi-alert';
    default:
      return 'i-bi-copy';
  }
});

const iconClassName = computed(() => {
  switch (copyState.value) {
    case 'copied':
      return 'text-green-500';
    case 'error':
      return 'text-red-500';
    default:
      return '';
  }
});

async function copyToClipboard () {
  if (copyState.value === 'copying') {
    return;
  }
  copyState.value = 'copying';
  try {
    if (navigator.clipboard && window.isSecureContext) {
      await navigator.clipboard.writeText(props.text);
    } else {
      // Fallback for insecure context or unsupported clipboard API
      const textarea = document.createElement('textarea');
      textarea.value = props.text;
      textarea.setAttribute('readonly', '');
      textarea.style.position = 'absolute';
      textarea.style.left = '-9999px';
      document.body.appendChild(textarea);
      textarea.select();
      document.execCommand('copy');
      document.body.removeChild(textarea);
    }
    copyState.value = 'copied';
    setTimeout(() => {
      copyState.value = 'idle';
    }, 1500);
  } catch (error) {
    copyState.value = 'error';
    console.error('Failed to copy to clipboard:', error);
    toast.add({
      id: 'copy-error',
      title: 'Failed to copy to clipboard',
      description: 'Check console for more details',
      icon: 'i-bi-alert',
      color: 'error',
      duration: 5000
    });
    setTimeout(() => {
      copyState.value = 'idle';
    }, 1500);
  }
}
</script>
